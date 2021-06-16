#include "FastaData.hpp"
#include <bifrost/CompactedDBG.hpp>

FastaData::FastaData(std::string ifpath_) : ifpath(ifpath_), size(0)
{
    FastqFile ffile({ ifpath_ });

    int read = ffile.read_next();
    while (read >= 0) {
        const kseq_t* fdata = ffile.get_kseq();

        seqs.push_back(fdata->seq.s);
        names.push_back(fdata->name.s);
        if (fdata->comment.s != NULL)
            comments.push_back(fdata->comment.s);
        size += seqs.back().length();

        read = ffile.read_next();
    }
}
