import sys
import json

def read_clust(stdin=False):
    clust = []

    if stdin:
        source = sys.stdin
    else:
        source = open(sys.argv[1])

    for line in source:
        s = line.strip()
        if s[0] == ">":
            number, size = [int(x) for x in s[1:].split()]
            c = { "size": size, "sketches": [] }
            clust.append(c)
        else:
            clust[-1]["sketches"].append(line.strip())

    d = { }
    for i, c in enumerate(clust):
        d[i] = c

    return d

def main():
    if not sys.stdin.isatty():
        clust = read_clust(stdin=True)
    else:
        clust = read_clust()

    with open('out.json', 'w') as f:
        json.dump(clust, f, indent = 4)

if __name__ == "__main__":
    main()
