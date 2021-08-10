#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <cctype>

std::vector<std::string> split(std::string line)
{
    std::vector<std::string> tokens;
    std::istringstream iss(line);
    std::string token;

    while(std::getline(iss, token, '\t'))
        tokens.push_back(token);

    return tokens;
}

struct TsvFile
{
    std::vector<std::vector<int>> bit_matrix;
    std::vector<std::string> info;

    TsvFile() = default;

    TsvFile(std::string fname)
    {
        read(fname);
    }

    void read(std::string fname)
    {
        std::fstream fs(fname);
        std::string line;
        std::getline(fs, line);

        auto tokens = split(line);
        for (int i = 1; i < tokens.size(); i++)
            info.push_back(tokens[i]);

        while (std::getline(fs, line))
        {
            tokens = split(line);
            bit_matrix.push_back({ });
            for (int i = 1; i < tokens.size(); i++)
                bit_matrix.back().push_back(std::stoi(tokens[i]));
        }
    }
};

std::vector<int> findValidFeatures(TsvFile& tsv)
{
    std::vector<int> valid;

    for (int i = 0; i < tsv.bit_matrix.size(); i++)
    {
        for (auto x : tsv.bit_matrix[i])
        {
            if (x == 0)
            {
                valid.push_back(i);
            }
        }
    }

    return valid;
}

int main(int argc, char** argv)
{
    TsvFile tsv(argv[1]);
    auto valid = findValidFeatures(tsv);
}
