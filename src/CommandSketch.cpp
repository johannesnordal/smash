#include <filesystem>
#include <getopt.h>
#include "Sketch.hpp"

void print_usage()
{
    static char const s[] = "Usage: sketch [options] <input> [<input>]\n\n"
        "Options:\n"
        "  -k <i32>    Size of kmers [default: 21]\n"
        "  -c <i32>    Candidate set limit [default: 1]\n"
        "  -s <i32>    Size of min hash [default: 1000]\n"
        "  -d <path>   Destination directory for sketch(s)\n"
        "  -j          Write sketch(s) to JSON as well\n"
        "  -O          Only write sketch(s) to JSON\n"
        "  -h          Show this screen.\n";
    printf("%s\n", s);
}

inline void concurrent(const char* ifpath)
{
    Sketch{ifpath};
}

int main(int argc, char** argv)
{
    if (argc == 1) {
        print_usage();
        exit(1);
    }

    MinHash::set_k(21);
    MinHash::set_c(1);
    MinHash::set_s(1000);
    MinHash::set_seed(0);

    int option;
    while ((option = getopt(argc, argv, "k:c:s:S:d:o:hjO")) != -1)
    {
        switch (option)
        {
            case 'k' :
                MinHash::set_k(atoi(optarg));
                break;
            case 'c' :
                MinHash::set_c(atoi(optarg));
                break;
            case 's' :
                MinHash::set_s(atoi(optarg));
                break;
            case 'S' :
                MinHash::set_seed(atoi(optarg));
                break;
            case 'd' :
                Sketch::ofpath = optarg;
                break;
            case 'j' :
                Sketch::write_json = true;
                break;
            case 'O' :
                Sketch::write_only_json = true;
                break;
            case 'h' :
                print_usage();
                exit(0);
        }
    }

    for (; optind < argc; optind++) {
        Sketch{argv[optind]};
    }

    /*
    if (argc < 3) {
        Sketch{argv[optind]};
        exit(0);
    }

    std::vector<std::thread*> threads;
    for (; optind < argc; optind++) {
        thread* t = new thread(concurrent, argv[optind]);
        threads.push_back(t);
    }

    for (auto t : threads)
        t->join();

    for (auto t : threads)
        delete t;
        */
}
