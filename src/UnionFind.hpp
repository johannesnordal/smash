#ifndef UNIONFIND_HPP
#define UNIONFIND_HPP

class UnionFind
{
    int* parent;
    int* rank;
    size_t m_components;
    size_t m_size;

    public:

    UnionFind(size_t n) : m_components{n}, m_size{n}
    {
        parent = new int[n];

        for (int i = 0; i < n; i++)
        {
            parent[i] = i;
        }

        rank = new int[n]();
    }

    ~UnionFind()
    {
        delete[] parent;
        delete[] rank;
    }

    int find(int x)
    {
        if (x != parent[x])
        {
            parent[x] = find(parent[x]);
        }

        return parent[x];
    }

    void merge(int x, int y)
    {
        if (m_components > 0)
            m_components--;

        int p = find(x);
        int q = find(y);

        if (rank[p] > rank[q])
        {
            parent[q] = p;
        }
        else
        {
            parent[p] = q;

            if (rank[p] == rank[q])
            {
                rank[q]++;
            }
        }
    }

    inline size_t size()
    {
        return m_size;
    }

    inline size_t components()
    {
        return m_components;
    }
};

#endif
