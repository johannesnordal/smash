#include <getopt.h>
#include "Sketch.hpp"
#include "MinHash.hpp"

Sketch::Sketch(std::string ifpath) : fdata{ifpath}
{
    min_hash(fdata.seqs);

    if (write_txt) {
      txt(ifpath);
      return;
    }

    if (write_only_json) {
        json(ifpath);
        return;
    }

    if (write_json)
        json(ifpath);

    write(ifpath);
}

void Sketch::write(const std::string& ifpath) const
{
    std::string ifname = ifpath.substr(ifpath.find_last_of("\\/") + 1);
    std::string ofname = ifname + ".sketch";

    std::ofstream fout(ofpath + ofname);

    uint32_t u32;
    uint64_t u64;

    u32 = ifpath.length();
    fout.write((char*) &u32, sizeof(u32));
    fout.write(ifpath.c_str(), ifpath.length());

    u32 = MinHash::k;
    fout.write((char*) &u32, sizeof(u32));

    u32 = MinHash::c;
    fout.write((char*) &u32, sizeof(u32));

    u32 = MinHash::s;
    fout.write((char*) &u32, sizeof(u32));

    u64 = MinHash::seed;
    fout.write((char*) &u64, sizeof(u64));

    u32 = fdata.size;
    fout.write((char*) &u32, sizeof(u32));

    for (size_t i = 0; i < min_hash.size(); i++) {
        u64 = min_hash[i];
        fout.write((char*) &u64, sizeof(u64));
    }

    fout.close();
}

void Sketch::txt(const std::string& ifpath) const
{
    std::string ifname = ifpath.substr(ifpath.find_last_of("\\/") + 1);
    std::string ofname = ifname + ".sketch";

    std::ofstream fout(ofpath + ofname);

    fout << ifpath << "\n"
         << MinHash::k << "\n"
         << MinHash::c << "\n"
         << MinHash::s << "\n";
    for (size_t i = 0; i < min_hash.size(); i++) {
      fout << min_hash[i] << "\n";
    }

    fout.close();
}

SketchData Sketch::read(const char* fpath)
{
    SketchData sdata;

    std::ifstream fin (fpath, std::ifstream::binary);
    std::string buf;

    char byte;
    while (fin.get(byte))
        buf.push_back(byte);

    const char* bytes = buf.c_str();
    const char* end = bytes + buf.size();

    uint32_t ifpath_len = *(uint32_t*) bytes;
    bytes += sizeof(ifpath_len);

    char *s = (char*) malloc(ifpath_len + 1);
    int i;
    for (i = 0; i < ifpath_len; i++)
      s[i] = bytes[i];
    s[i] = '\0';
    sdata.ifpath = s;
    bytes += ifpath_len;

    sdata.k = *(uint32_t*) bytes;
    bytes += sizeof(sdata.k);

    sdata.c = *(uint32_t*) bytes;
    bytes += sizeof(sdata.c);

    sdata.s = *(uint32_t*) bytes;
    bytes += sizeof(sdata.s);

    sdata.seed = *(uint64_t*) bytes;
    bytes += sizeof(sdata.seed);

    sdata.size = *(uint32_t*) bytes;
    bytes += sizeof(sdata.size);

    uint64_t hash;
    for ( ; bytes != end; bytes += sizeof(hash)) {
        hash = *(uint64_t*) bytes;
        sdata.min_hash.push_back(hash);
    }

    fin.close();

    return sdata;
}

void Sketch::json(const std::string& ifpath) const
{
    std::ofstream file;

    std::string ifname = ifpath.substr(ifpath.find_last_of("\\/") + 1);
    std::string ofname = ifname + ".sketch.json";

    file.open(ofpath + ofname);

    file << "{\n";

    if (min_hash.size() == MinHash::s) {
        file << "\t\"error\": [],\n";
    } else {
        file << "\t\"error\": [\n";
        file << "\t\t\"min hash isn't full\"\n";
        file << "\t],\n";
    }

    file << "\t\"file\": " << "\"" << ifpath << "\",\n";

    file << "\t\"k\": " << MinHash::k << ",\n";
    file << "\t\"c\": " << MinHash::c << ",\n";
    file << "\t\"s\": " << MinHash::s << ",\n";
    file << "\t\"seed\": " << MinHash::seed << ",\n";
    file << "\t\"size\": " << fdata.size << ",\n";

    file << "\t\"minhash\": [\n";

    for (size_t i = 0; i < min_hash.size() - 1; i++)
        file << "\t\t" << min_hash[i] << ",\n";

    file << "\t\t" << min_hash[min_hash.size() - 1] << "\n";

    file << "\t]\n}";
    file.close();
}

std::string Sketch::ofpath = "";
bool Sketch::write_json = false;
bool Sketch::write_only_json = false;
bool Sketch::write_txt = false;
