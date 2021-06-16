#ifndef FASTADATA_H
#define FASTADATA_H
#include <bifrost/CompactedDBG.hpp>

struct FastaData {
    std::vector<std::string> seqs;
    std::vector<std::string> names;
    std::vector<std::string> comments;
    std::string ifpath;
    size_t size;

    FastaData(std::string ifpath);
};

#endif
