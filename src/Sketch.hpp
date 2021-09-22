#ifndef SKETCH_HPP
#define SKETCH_HPP
#include "MinHash.hpp"
#include "FastaData.hpp"

struct SketchData {
    uint32_t k;
    uint32_t c;
    uint32_t s;
    uint32_t size;
    uint64_t seed;
    std::string ifpath;
    std::vector<uint64_t> min_hash;
};

struct Sketch {
    FastaData fdata;
    MinHash min_hash;

    Sketch(std::string);
    Sketch(FastaData);
    void write(const std::string&) const;
    void txt(const std::string&) const;
    void json(const std::string&) const;

    static std::string ofpath;
    static bool write_txt;
    static bool write_json;
    static bool write_only_json;
    static SketchData read(const char*);
};

#endif
