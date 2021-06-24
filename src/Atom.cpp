#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include "Sketch.hpp"
#include "UnionFind.hpp"
#include "khashl.hpp"

using Table = klib::KHashMap<uint64_t, std::vector<size_t>*, std::hash<uint64_t>>;

Table make_table(std::vector<SketchData>& sketch_list)
{
    Table table;

    for (size_t i = 0; i < sketch_list.size(); i++)
    {
        SketchData& sketch = sketch_list[i];
        for (uint64_t hash : sketch.minhash)
        {
            if (table.get(hash) != table.end())
            {
                table[hash]->push_back(i);
            }
            else
            {
                table[hash] = new std::vector<size_t>;
                table[hash]->push_back(i);
            }
        }
    }

    return table;
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

using Clust = klib::KHashMap<size_t, std::vector<size_t>*, std::hash<size_t>>;

Clust make_clusters(const std::vector<SketchData>& sketch_list,
                    Table& table,
                    const size_t limit)
{
    UnionFind uf{sketch_list.size()};

    for (size_t i = 0; i < sketch_list.size(); i++)
    {
        // Indices of sketches and number of mutual hash values.
        klib::KHashMap<size_t, size_t, std::hash<size_t>> mutual;

        for (auto hash : sketch_list[i].minhash)
        {
            // Indices of sketches where hash appears.
            const auto sketch_indices = table[hash];

            for (auto j : *sketch_indices)
            {
                if (i == j) continue;
                if (mutual.get(j) != mutual.end())
                    mutual[j] += 1;
                else
                    mutual[j] = 1;
            }
        }

        for (size_t k = 0; k != mutual.end(); ++k)
        {
            if (mutual.occupied(k))
            {
                const auto j = mutual.key(k);
                const auto c = mutual.value(k);

                if (c > limit && uf.find(i) != uf.find(j))
                    uf.merge(i, j);
            }
        }
    }

    Clust cl;
    for (size_t x = 0; x < sketch_list.size(); x++)
    {
        const size_t parent = uf.find(x);

        if (uf.size(parent) > 1)
        {
            if (cl.get(parent) != cl.end())
            {
                cl[parent]->push_back(x);
            }
            else
            {
                cl[parent] = new std::vector<size_t>;
                cl[parent]->push_back(x);
            }

        }
    }

    return cl;
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

    auto table = make_table(sketch_list);

    size_t limit = 990;
    auto clust = make_clusters(sketch_list, table, limit);

    size_t i = 0;
    for (size_t k = 0; k != clust.end(); ++k)
    {
        if (clust.occupied(k))
        {
            auto val = clust.value(k);

            std::cout << ">" << i << " " << val->size() << "\n";
            for (auto x : *val)
            {
                std::cout << fnames[x] << "\n";
            }

            delete val;

            i++;
        }
    }

    for (size_t k = 0; k != table.end(); ++k)
        if (table.occupied(k))
            delete table.value(k);
}
