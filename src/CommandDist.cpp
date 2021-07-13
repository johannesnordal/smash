#include <getopt.h>
#include "Sketch.hpp"
#include "Dist.hpp"

void print_usage()
{
    static char const s[] = "Usage: dist <sketch> <sketch>\n\n"
        "Options:\n"
        "   -h  Show this screen\n";

    printf("%s\n", s);
}

int main(int argc, char** argv) {
    if (argc < 3) {
        print_usage();
        exit(1);
    }

    int option;
    while ((option = getopt(argc, argv, "h")) != -1) {
        switch (option)
        {
            case 'h' :
                print_usage();
                exit(0);
        }
    }

    auto s1 = Sketch::read(argv[optind]);
    auto s2 = Sketch::read(argv[optind + 1]);

    double k = (double) s1.k;
    double sketchSize = std::min(s1.s, s2.s);
    double sharedK = shared_kmers(s1.min_hash, s2.min_hash, sketchSize);
    double Jaccard = sharedK/sketchSize;;
    double mashDistance = computeDist(k, Jaccard);
    double rValue = computeRValue(k, s1.size, s2.size);
    double pValue = computePValue(sharedK, sketchSize, rValue);

    std::cout << argv[optind] << "\t";
    std::cout << argv[optind + 1] << "\t";
    std::cout << mashDistance << "\t";
    std::cout << pValue << "\t";
    std::cout << sharedK<< "/" <<sketchSize << "\t";
    std::cout << k << "\t";
    std::cout << sketchSize << std::endl;
}
