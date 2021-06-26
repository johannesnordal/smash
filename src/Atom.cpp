#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include <klib/khash.h>
// #include "khash.h"
#include "Sketch.hpp"
#include "UnionFind.hpp"

KHASH_MAP_INIT_INT64(vec64, std::vector<uint64_t>*);
KHASH_MAP_INIT_INT64(u64, uint64_t);

khash_t(vec64)* make_table(std::vector<SketchData>& sketch_list)
{
    int ret;
    khiter_t k;
    khash_t(vec64)* m_table = kh_init(vec64);

    for (uint64_t i = 0; i < sketch_list.size(); i++)
    {
        SketchData& sketch = sketch_list[i];
        for (uint64_t hash : sketch.minhash)
        {
            k = kh_put(vec64, m_table, hash, &ret);

            if (ret)
            {
                kh_value(m_table, k) = new std::vector<uint64_t>;
            }

            kh_value(m_table, k)->push_back(i);
        }
    }

    return m_table;
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

UnionFind make_clusters(const std::vector<SketchData>& sketch_list,
        khash_t(vec64)* m_table, const uint64_t limit)
{
    int code;
    khiter_t k;

    UnionFind uf(sketch_list.size());

    for (uint64_t i = 0; i < sketch_list.size(); i++)
    {
        // Indices of sketches and number of mutual hash values.
        khash_t(u64)* mutual = kh_init(u64);

        for (auto hash : sketch_list[i].minhash)
        {
            // Indices of sketches where hash appears.
            k = kh_get(vec64, m_table, hash);
            std::vector<uint64_t>* sketch_indices = kh_value(m_table, k);

            for (auto j : *sketch_indices)
            {
                if (i == j) continue;

                k = kh_get(u64, mutual, j);

                if (k != kh_end(mutual))
                {
                    kh_value(mutual, k) += 1;
                }
                else
                {
                    k = kh_put(u64, mutual, j, &code);
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

    return uf;
}

int main(int argc, char** argv)
{
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

    auto m_table = make_table(sketch_list);

    uint64_t limit = 990;
    auto uf = make_clusters(sketch_list, m_table, limit);

    khiter_t k;

    printf("cluster,filename\n");

    int parent;
    for (int i = 0; i < fnames.size(); i++)
    {
        parent = uf.find(i);
        printf("%d,%s\n", parent, fnames[i].c_str());
    }

    for (k = kh_begin(m_table); k != kh_end(m_table); ++k)
    {
        if (kh_exist(m_table, k))
        {
            delete kh_value(m_table, k);
        }
    }

    kh_destroy(vec64, m_table);
}
