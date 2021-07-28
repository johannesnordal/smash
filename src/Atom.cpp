#include "khash.h"
#include "Sketch.hpp"
#include "Dist.hpp"
#include "UnionFind.hpp"
#include <getopt.h>
#include <cstdint>
#include <algorithm>
#include <postgresql/libpq-fe.h>

KHASH_MAP_INIT_INT64(vector_u64, std::vector<uint64_t>*);
KHASH_MAP_INIT_INT64(u64, uint64_t);

khash_t(vector_u64)* make_hash_locator(std::vector<SketchData>& sketch_list)
{
    int ret;
    khiter_t k;
    khash_t(vector_u64)* hash_locator = kh_init(vector_u64);

    for (uint64_t i = 0; i < sketch_list.size(); i++)
    {
        SketchData& sketch = sketch_list[i];
        for (uint64_t hash : sketch.min_hash)
        {
            k = kh_get(vector_u64, hash_locator, hash);

            if (k == kh_end(hash_locator))
            {
                k = kh_put(vector_u64, hash_locator, hash, &ret);
                kh_value(hash_locator, k) = new std::vector<uint64_t>;
            }

            kh_value(hash_locator, k)->push_back(i);
        }
    }

    return hash_locator;
}

std::vector<std::string> read(std::string ifpath)
{
    std::vector<std::string> fnames;
    std::fstream fin;

    fin.open(ifpath, std::ios::in);

    if (fin.is_open())
    {
        std::string fname;

        while (getline(fin, fname))
        {
            fnames.push_back(fname);
        }
    }

    return fnames;
}

khash_t(vector_u64)* make_clusters(const std::vector<SketchData>& sketch_list,
        khash_t(vector_u64)* hash_locator, const uint64_t limit)
{
    int ret;
    khiter_t k;

    UnionFind uf(sketch_list.size());

    for (uint64_t i = 0; i < sketch_list.size(); i++)
    {
        // Indices of sketches and number of mutual hash values.
        khash_t(u64)* mutual = kh_init(u64);

        for (auto hash : sketch_list[i].min_hash)
        {
            // Indices of sketches where hash appears.
            k = kh_get(vector_u64, hash_locator, hash);
            std::vector<uint64_t>* sketch_indices = kh_value(hash_locator, k);

            for (auto j : *sketch_indices)
            {
                k = kh_get(u64, mutual, j);

                if (k != kh_end(mutual))
                {
                    kh_value(mutual, k) += 1;
                }
                else
                {
                    k = kh_put(u64, mutual, j, &ret);
                    kh_value(mutual, k) = 1;
                }
            }
        }

        for (k = kh_begin(mutual); k != kh_end(mutual); ++k)
        {
            if (kh_exist(mutual, k))
            {
                const auto j = kh_key(mutual, k);
                const auto c = kh_value(mutual, k);

                if (c > limit && uf.find(i) != uf.find(j))
                {
                    uf.merge(i, j);
                }
            }
        }

        kh_destroy(u64, mutual);
    }

    khash_t(vector_u64)* clusters = kh_init(vector_u64);

    for (int x = 0; x < uf.size(); x++)
    {
        const int parent = uf.find(x);

        k = kh_get(vector_u64, clusters, parent);

        if (k == kh_end(clusters))
        {
            k = kh_put(vector_u64, clusters, parent, &ret);
            kh_value(clusters, k) = new std::vector<uint64_t>;
        }

        kh_value(clusters, k)->push_back(x);
    }

    return clusters;
}

std::vector<uint64_t> make_rep_sketch(std::vector<uint64_t>* cluster,
        std::vector<SketchData>& sketch_list)
{
    int ret;
    khiter_t k;
    khash_t(u64)* hash_counter = kh_init(u64);
    for (auto i : *cluster)
    {
        for (auto hash : sketch_list[i].min_hash)
        {
            k = kh_get(u64, hash_counter, hash);

            if (k == kh_end(hash_counter))
            {
                k = kh_put(u64, hash_counter, hash, &ret);
                kh_value(hash_counter, k) = 0;
            }

            kh_value(hash_counter, k) += 1;
        }
    }

    using Pair = std::pair<uint64_t, uint64_t>;
    const auto cmp = [](const Pair x, const Pair y) -> const bool {
        return x.second > y.second;
    };

    std::vector<std::pair<uint64_t, uint64_t>> hash_heap;
    for (k = kh_begin(hash_counter); k != kh_end(hash_counter); ++k)
    {
        if (kh_exist(hash_counter, k))
        {
            const auto hash = kh_key(hash_counter, k);
            const auto count = kh_value(hash_counter, k);

            hash_heap.push_back(std::make_pair(hash, count));

            // Need to add kmer-size variable
            if (hash_heap.size() == 1000)
            {
                std::make_heap(hash_heap.begin(), hash_heap.end(), cmp);
                break;
            }
        }
    }

    for ( ; k != kh_end(hash_counter); ++k)
    {
        if (kh_exist(hash_counter, k))
        {
            const auto hash = kh_key(hash_counter, k);
            const auto count = kh_value(hash_counter, k);
            if (count > hash_heap[0].second)
            {
                std::pop_heap(hash_heap.begin(), hash_heap.end(), cmp);
                hash_heap.pop_back();
                hash_heap.push_back(std::make_pair(hash, count));
                std::push_heap(hash_heap.begin(), hash_heap.end(), cmp);
            }
        }
    }

    std::vector<uint64_t> rep;
    for (auto x : hash_heap)
    {
        rep.push_back(x.first);
    }

    std::sort(rep.begin(), rep.end());

    return rep;
}

std::vector<std::vector<uint64_t>> make_reps(khash_t(vector_u64)* clusters,
        std::vector<SketchData>& sketch_list)
{
    std::vector<std::vector<uint64_t>> reps;

    for (khiter_t k = kh_begin(clusters); k != kh_end(clusters); ++k)
    {
        if (kh_exist(clusters, k))
        {
            auto cluster = kh_value(clusters, k);
            if (cluster->size() > 1)
            {
                reps.push_back(make_rep_sketch(cluster, sketch_list));
            }
        }
    }

    return reps;
}

void usage()
{
    static char const s[] = "Usage: atom [options] <file>\n\n"
        "Options:\n"
        "   -l <u64>    Mininum of mutual k-mers [default: 995/1000]\n"
        "   -h          Show this screen\n";
    std::printf("%s\n", s);
}

static void exit_nicely(PGconn *conn)
{
    PQfinish(conn);
    exit(1);
}

int main(int argc, char** argv)
{
    const char* conninfo = NULL;
    uint64_t limit = 995;

    int option;
    while ((option = getopt(argc, argv, "l:h")) != -1)
    {
        switch (option)
        {
            case 'l':
                limit = std::atoi(optarg);
                break;
            case 'h':
                usage();
                exit(0);
            case 'd':
                conninfo = optarg;
                break;
        }
    }

    if (optind == argc)
    {
        std::fprintf(stderr, "Error: Missing input file\n\n");
        usage();
        exit(1);
    }

    if (conninfo == NULL)
    {
        std::fprintf(stderr, "Error: Missing database connection string\n\n");
        usage();
        exit(1);
    }

    std::vector<SketchData> sketch_list;
    {
        std::vector<std::string> fnames = read(argv[optind]);
        sketch_list.reserve(fnames.size());
        for (auto fname : fnames)
        {
            sketch_list.push_back(Sketch::read(fname.c_str()));
        }
    }

    auto hash_locator = make_hash_locator(sketch_list);
    auto clusters = make_clusters(sketch_list, hash_locator, limit);
    auto reps = make_reps(clusters, sketch_list);

    PGconn *conn = PQconnectdb(conninfo);

    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Error: Connection to database failed: %s",
                PQerrorMessage(conn));
        exit_nicely(conn);
    }
}
