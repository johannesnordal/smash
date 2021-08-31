#include "khash.h"
#include "Sketch.hpp"
#include "Dist.hpp"
#include "UnionFind.hpp"
#include <getopt.h>
#include <cstdint>
#include <algorithm>
#define BYTE_FILE 0
#define CLUSTER_LIMIT_ERROR 5

using Pair = std::pair<uint64_t, uint64_t>;
const bool cmp(const Pair x, const Pair y)
{
    return x.second > y.second;
}

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

std::vector<uint64_t>* make_rep_sketch(std::vector<uint64_t>* cluster,
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

    std::vector<Pair> hash_heap;
    for (k = kh_begin(hash_counter); k != kh_end(hash_counter); ++k)
    {
        if (kh_exist(hash_counter, k))
        {
            const auto hash = kh_key(hash_counter, k);
            const auto count = kh_value(hash_counter, k);

            hash_heap.push_back(std::make_pair(hash, count));

            // TODO add minhash-size variable
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

    std::vector<uint64_t>* rep = new std::vector<uint64_t>;
    for (auto x : hash_heap)
    {
        rep->push_back(x.first);
    }

    std::sort(rep->begin(), rep->end());

    return rep;
}

khash_t(vector_u64)* make_reps(khash_t(vector_u64)* clusters,
        std::vector<SketchData>& sketch_list)
{
    khash_t(vector_u64)* reps = kh_init(vector_u64);

    for (khiter_t k = kh_begin(clusters); k != kh_end(clusters); ++k)
    {
        if (kh_exist(clusters, k))
        {
            auto parent = kh_key(clusters, k);
            auto cluster = kh_value(clusters, k);
            if (cluster->size() > 1)
            {
                int ret;
                khiter_t k = kh_put(vector_u64, reps, parent, &ret);
                kh_value(reps, k) = make_rep_sketch(cluster, sketch_list);
            }
        }
    }

    return reps;
}

uint32_t find_cluster_limit(std::vector<uint64_t>* cluster,
        std::vector<uint64_t>* rep,
        std::vector<SketchData>& sketch_list,
        uint64_t limit)
{
    uint64_t dist = limit;

    for (auto i : *cluster)
    {
        std::vector<uint64_t> diff(rep->size());
        std::vector<uint64_t>::iterator it;
        auto mem = sketch_list[i].min_hash;
        it = std::set_difference(mem.begin(), mem.end(), rep->begin(),
                rep->end(), diff.begin()); 
        diff.resize(it - diff.begin());
        if (limit - diff.size() < dist)
            dist = limit - diff.size();
    }

    return dist - CLUSTER_LIMIT_ERROR;
}

void fwrite_rep(std::vector<uint64_t>* rep, uint64_t limit,
        std::string fname)
{
    std::string data;
    std::ofstream fout(fname);

#if BYTE_FILE
    data = std::to_string(limit);
    fout.write(data.c_str(), sizeof(data));

    for (auto hash : *rep)
    {
        data = std::to_string(hash);
        fout.write(data.c_str(), sizeof(data));
    }
#else
    fout << limit << "\n";

    for (auto hash : *rep)
    {
        fout << hash << "\n";
    }
#endif

    fout.close();
}

void usage()
{
    static char const s[] = "Usage: atom [options] <file>\n\n"
        "Options:\n"
        "   -l <u64>    Mininum of mutual k-mers [default: 995/1000].\n"
        "   -r          Rep sketch path\n"
        "   -i          Info file name\n"
        "   -h          Show this screen.\n";
    std::printf("%s\n", s);
}

int main(int argc, char** argv)
{
    uint64_t limit = 995;
    std::string rep_path = "";
    std::string info_file = "";

    int option;
    while ((option = getopt(argc, argv, "l:r:i:h")) != -1)
    {
        switch (option)
        {
            case 'l':
                limit = std::atoi(optarg);
                break;
            case 'r':
                rep_path = optarg;
                break;
            case 'i':
                info_file = optarg;
                break;
            case 'h':
                usage();
                exit(0);
        }
    }

    if (optind == argc)
    {
        std::fprintf(stderr, "Error: Missing input file\n\n");
        usage();
        exit(1);
    }

    if (rep_path == "")
    {
        std::fprintf(stderr, "Error: missing rep sketch path\n\n");
        usage();
        exit(1);
    }

    if (info_file == "")
    {
        std::fprintf(stderr, "Error: missing info file name\n\n");
        usage();
        exit(1);
    }

    std::vector<SketchData> sketch_list;
    std::vector<std::string> fnames = read(argv[optind]);
    sketch_list.reserve(fnames.size());
    for (auto fname : fnames)
    {
        sketch_list.push_back(Sketch::read(fname.c_str()));
    }

    auto hash_locator = make_hash_locator(sketch_list);
    auto clusters = make_clusters(sketch_list, hash_locator, limit);
    auto reps = make_reps(clusters, sketch_list);

    for (khiter_t k = kh_begin(reps); k != kh_end(reps); ++k)
    {
        if (kh_exist(reps, k))
        {
            auto clust_idx = kh_key(reps, k);
            auto rep = kh_value(reps, k);
            uint64_t rep_limit;
            {
                khiter_t k = kh_get(vector_u64, clusters, clust_idx);
                auto cluster = kh_value(clusters, k);
                rep_limit = find_cluster_limit(cluster, rep, sketch_list,
                        limit);
            }
            fwrite_rep(rep, rep_limit, rep_path + std::to_string(clust_idx));
        }
    }

#if 0
    std::ofstream fout(info_file);
    fout << "cluster,members,size\n";
    for (khiter_t k = kh_begin(clusters); k != kh_end(clusters); ++k)
    {
        if (kh_exist(clusters, k))
        {
            auto cluster = kh_key(clusters, k);
            auto members = kh_val(clusters, k);

            fout << cluster << ",";

            int i;
            for (i = 0; i < members->size() - 1; i++)
            {
                fout << fnames[(*members)[i]] << ";";
            }

            fout << fnames[(*members)[i]] << "," << members->size() << "\n";
        }
    }
    fout.close();
#endif

    std::vector<Pair> cluster_size;
    for (khiter_t k = kh_begin(clusters); k != kh_end(clusters); ++k)
    {
        if (kh_exist(clusters, k))
        {
            auto cluster = kh_key(clusters, k);
            auto size = kh_value(clusters, k)->size();
            cluster_size.push_back(std::make_pair(cluster, size));
        }
    }
    printf("%ld\n", cluster_size.size());
    std::sort(cluster_size.begin(), cluster_size.end(), cmp);

    std::ofstream fout(info_file);
    fout << "cluster,size,members\n";
    for (auto p : cluster_size)
    {
        khiter_t k = kh_get(vector_u64, clusters, p.first);
        auto members = kh_value(clusters, k);

        fout << p.first << "," << p.second << ",";

        int i;
        for (i = 0; i < members->size() - 1; i++)
        {
            fout << fnames[(*members)[i]] << ";";
        }

        fout << fnames[(*members)[i]] << "\n";
    }
    fout.close();
}
