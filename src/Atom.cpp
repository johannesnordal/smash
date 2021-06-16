#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <utility>
#include "Sketch.hpp"

std::map<uint64_t, std::vector<std::string>> make_table(
        std::vector<SketchData>& sketch_list,
        std::vector<std::string> path)
{
    std::map<uint64_t, std::vector<std::string>> map;

    for (int i = 0; i < sketch_list.size(); i++)
    {
        SketchData& sketch = sketch_list[i];
        for (uint64_t x : sketch.minhash)
        {
            auto it = map.find(x);

            if (it != map.end())
            {
                it->second.push_back(path[i]);
            }
            else
            {
                const std::pair<uint64_t, std::vector<std::string>> entry(x, { path[i] });
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

    auto table = make_table(sketch_list, fnames);

    for (auto entry : table)
    {
        if (entry.second.size() > 2)
        {
            std::cout << entry.first << std::endl;

            for (auto hashmer : entry.second)
            {
                std::cout << hashmer << std::endl;
            }

            std::cout << std::endl;
        }
    }
}
