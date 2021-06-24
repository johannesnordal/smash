#ifndef UNIONFIND_HPP
#define UNIONFIND_HPP

class UnionFind
{
    int* parent;
    int* rank;
    int* set_size;

    public:

    UnionFind(size_t n)
    {
        parent = new int[n];
        set_size = new int[n];

        for (int i = 0; i < n; i++)
        {
            parent[i] = i;
            set_size[i] = 1;
        }

        rank = new int[n]();
    }

    ~UnionFind()
    {
        delete[] parent;
        delete[] rank;
        delete[] set_size;
    }

    int find(int x)
    {
        while (x != parent[x])
        {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }

        return x;
    }

    inline int size(int x)
    {
        return set_size[find(x)];
    }

    void merge(const int x, const int y)
    {
        int p = find(x);
        int q = find(y);

        if (p == q)
        {
            return;
        }

        if (rank[p] > rank[q])
        {
            parent[q] = p;
            set_size[p] += set_size[q];
        }
        else
        {
            parent[p] = q;
            set_size[q] += set_size[p];

            if (rank[p] == rank[q])
            {
                rank[q]++;
            }
        }
    }
};

#endif
