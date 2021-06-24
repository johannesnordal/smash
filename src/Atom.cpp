#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include "Sketch.hpp"
#include "UnionFind.hpp"

std::map<uint64_t, std::vector<size_t>> make_table(std::vector<SketchData>& sketch_list)
{
    std::map<uint64_t, std::vector<size_t>> table;

    for (size_t i = 0; i < sketch_list.size(); i++)
    {
        SketchData& sketch = sketch_list[i];
        for (uint64_t hash : sketch.minhash)
        {
            if (table.find(hash) != table.end())
            {
                table[hash].push_back(i);
            }
            else
            {
                table[hash] = { i };
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

std::map<size_t, std::vector<size_t>>
make_clusters(const std::vector<SketchData>& sketch_list,
              const std::map<uint64_t, std::vector<size_t>>& table,
              const size_t limit)
{
    UnionFind uf{sketch_list.size()};

    for (size_t i = 0; i < sketch_list.size(); i++)
    {
        // Indices of sketches and number of mutual hash values.
        std::map<size_t, size_t> mutual;

        for (auto hash : sketch_list[i].minhash)
        {
            // Indices of sketches where hash appears.
            const auto sketch_indices = table.find(hash)->second;

            for (auto j : sketch_indices)
            {
                if (i == j) continue;
                if (mutual.find(j) != mutual.end())
                    mutual[j] += 1;
                else
                    mutual[j] = 1;
            }
        }

        for (auto entry : mutual)
        {
            if (entry.second > limit && uf.find(i) != uf.find(entry.first))
            {
                uf.merge(i, entry.first);
            }
        }
    }

    std::map<size_t, std::vector<size_t>> clusters;
    for (size_t x = 0; x < sketch_list.size(); x++)
    {
        const size_t parent = uf.find(x);

        if (uf.size(parent) > 1)
        {
            if (clusters.find(parent) != clusters.end())
                clusters[parent].push_back(x);
            else
                clusters[parent] = { x };
        }
    }

    return clusters;
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

    const size_t limit = 990;
    auto table = make_table(sketch_list);
    auto clust = make_clusters(sketch_list, table, limit);

    for (auto entry : clust)
    {
        std::cout << entry.first << "\n";
    }
}
