#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include "Sketch.hpp"
#include "UnionFind.hpp"

std::map<uint64_t, std::vector<size_t>>
make_table(std::vector<SketchData>& sketch_list)
{
    std::map<uint64_t, std::vector<size_t>> map;

    for (size_t i = 0; i < sketch_list.size(); i++)
    {
        SketchData& sketch = sketch_list[i];
        for (uint64_t hashmer : sketch.minhash)
        {
            auto it = map.find(hashmer);

            if (it != map.end())
            {
                it->second.push_back(i);
            }
            else
            {
                std::vector<size_t> match_list = { i };
                std::pair<uint64_t, std::vector<size_t>> entry;
                entry = std::make_pair(hashmer, match_list);
                map.insert(entry);
            }
        }
    }

    return map;
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

void make_clusters(std::vector<SketchData>& sketch_list,
                   std::map<uint64_t, std::vector<size_t>> table)
{
    UnionFind uf{sketch_list.size()};

    for (size_t i = 0; i < sketch_list.size(); i++)
    {
        std::map<size_t, size_t> mutual;

        for (auto hashmer : sketch_list[i].minhash)
        {
            std::vector<size_t> sketch_idx = table.find(hashmer)->second;

            for (auto j : sketch_idx)
            {
                auto it = mutual.find(j);

                if (it != mutual.end())
                {
                    it->second += 1;
                }
                else
                {
                    mutual.insert(
                }
            }
        }
    }
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
}
