#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <cstdlib>
#include <utility>
#include "khash.h"
#include "Sketch.hpp"
#include "UnionFind.hpp"

KHASH_MAP_INIT_INT64(vec64, std::vector<uint64_t>*);
KHASH_MAP_INIT_INT64(u64, uint64_t);

khash_t(vec64)* make_hash_locator(std::vector<SketchData>& sketch_list)
{
    int ret;
    khiter_t k;
    khash_t(vec64)* hash_locator = kh_init(vec64);

    for (uint64_t i = 0; i < sketch_list.size(); i++)
    {
        SketchData& sketch = sketch_list[i];
        for (uint64_t hash : sketch.minhash)
        {
            k = kh_get(vec64, hash_locator, hash);

            if (k == kh_end(hash_locator))
            {
                k = kh_put(vec64, hash_locator, hash, &ret);
                kh_value(hash_locator, k) = new std::vector<uint64_t>;
            }

            kh_value(hash_locator, k)->push_back(i);
        }
    }

    return hash_locator;
}

std::vector<std::string> read()
{
    std::vector<std::string> fnames;

    for (std::string fname; std::getline(std::cin, fname); )
    {
        fnames.push_back(fname);
    }

    return fnames;
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

khash_t(vec64)* make_clusters(const std::vector<SketchData>& sketch_list,
        khash_t(vec64)* hash_locator, const uint64_t limit)
{
    int ret;
    khiter_t k;

    UnionFind uf(sketch_list.size());

    for (uint64_t i = 0; i < sketch_list.size(); i++)
    {
        // Indices of sketches and number of mutual hash values.
        khash_t(u64)* mutual = kh_init(u64);

        for (auto hash : sketch_list[i].minhash)
        {
            // Indices of sketches where hash appears.
            k = kh_get(vec64, hash_locator, hash);
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

    khash_t(vec64)* clusters = kh_init(vec64);

    for (int x = 0; x < uf.size(); x++)
    {
        const int parent = uf.find(x);

        k = kh_get(vec64, clusters, parent);

        if (k == kh_end(clusters))
        {
            k = kh_put(vec64, clusters, parent, &ret);
            kh_value(clusters, k) = new std::vector<uint64_t>;
        }

        kh_value(clusters, k)->push_back(x);
    }

    return clusters;
}

#if 0
khash_t(u64)* make_reps(khash_t(vec64)* clusters,
        std::vector<SketchData>& sketch_list)
{

}
#endif

int main(int argc, char** argv)
{
    uint64_t limit = 995;
    std::vector<std::string> fnames;

    if (argc > 1)
        fnames = read(argv[1]);
    else
        fnames = read();

    std::vector<SketchData> sketch_list;
    sketch_list.reserve(fnames.size());

    for (auto fname : fnames)
    {
        sketch_list.push_back(Sketch::read(fname.c_str()));
    }

    auto hash_locator = make_hash_locator(sketch_list);
    auto clusters = make_clusters(sketch_list, hash_locator, limit);

    khiter_t k;
    for (k = kh_begin(clusters); k != kh_end(clusters); ++k)
    {
        if (kh_exist(clusters, k))
        {
            std::printf("%lu: ", kh_key(clusters, k));

            for (auto x : *kh_value(clusters, k))
            {
                std::printf("%lu ", x);
            }

            std::printf("\n");
        }
    }
}
