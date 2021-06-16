#ifndef MINHASH_H
#define MINHASH_H
#include <algorithm>
#include <vector>
#include "CandidateSet.hpp"
#include "FastaData.hpp"

class MinHash
{
    CandidateSet set;
    Kmer* min_hash;
    size_t m_size;

    void sink();

    public:

    MinHash() : min_hash{new Kmer[s]}, m_size{0} { }

    ~MinHash()
    {
        delete[] min_hash;
    }

    void operator()(const std::vector<string>& seqs);

    inline const uint64_t operator[](size_t i) const
    {
        return min_hash[i].hash();
    }

    inline size_t size() const
    {
        return m_size;
    }

    static uint32_t k, c, s;
    static uint64_t seed;

    static void set_k(uint32_t);
    static void set_c(uint32_t);
    static void set_s(uint32_t);
    static void set_seed(uint64_t);
};

#endif
