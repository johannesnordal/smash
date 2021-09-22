#include "MinHash.hpp"

uint32_t MinHash::k;
uint32_t MinHash::c;
uint32_t MinHash::s;
uint64_t MinHash::seed;

void MinHash::sink()
{
    size_t i = 0;
    size_t j = 1;

    while (j < m_size)
    {
        const size_t temp = j + 1;
        if (temp < m_size && min_hash[temp].hash() > min_hash[j].hash())
            j = temp;

        if (min_hash[i].hash() > min_hash[j].hash())
            return;

        const Kmer kmer = min_hash[i];
        min_hash[i] = min_hash[j];
        min_hash[j] = kmer;

        i = j;
        j = j << 1;
    }
}

void MinHash::set_k(uint32_t k_)
{
    Kmer::set_k(k_);

    k = k_;
}

void MinHash::set_c(uint32_t c_)
{
    c = c_;
}

void MinHash::set_s(uint32_t s_)
{
    s = s_;
}

void MinHash::set_seed(uint64_t seed_)
{
    seed = seed_;
}

void MinHash::operator()(const std::vector<string>& seqs)
{
    const uint64_t MAX_HASH = 9999999776999205UL;
    const KmerIterator it_end;

    const auto cmp = [](const Kmer x, const Kmer y) -> const bool {
        return x.hash() < y.hash();
    };

    if (s != 0)
    {
        for (const auto& seq : seqs)
        {
            KmerIterator it(seq.c_str());

            for (; m_size < s && it != it_end; ++it)
            {
                const Kmer kmer = it->first.rep();

                if (c == set.update(kmer)) {
                    min_hash[m_size] = kmer;
                    m_size += 1;
                }

                if (m_size == s)
                    std::make_heap(min_hash, min_hash + m_size, cmp);
            }

            for (; it != it_end; ++it)
            {
                const Kmer kmer = it->first.rep();

                if (min_hash[0].hash() > kmer.hash())
                {
                    if (c == set.update(kmer))
                    {
                        set.erase(min_hash[0]);
                        min_hash[0] = kmer;
                        sink();
                    }
                }
            }
        }
    }
    else
    {
        std::vector<Kmer> vec;
        for (const auto& seq : seqs)
        {
            KmerIterator it(seq.c_str());

            for (; it != it_end; ++it)
            {
                const Kmer kmer = it->first.rep();

                if (kmer.hash() < MAX_HASH) {
                    if (c == set.update(kmer))
                        vec.push_back(kmer);
                }
            }
        }

        // Kind of hacky
        delete[] min_hash;
        min_hash = new Kmer[vec.size()];

        for (size_t i = 0; i < vec.size(); i++)
            min_hash[i] = vec[i];

        m_size = vec.size();
        MinHash::set_s(m_size);
    }

    std::sort(min_hash, min_hash + m_size, cmp);
}
