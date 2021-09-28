#include <getopt.h>
#include "Sketch.hpp"

void print_usage()
{
    static char const s[] = "Usage: sketch [options] <input> [<input>]\n\n"
        "Options:\n"
        "  -k <i32>    Size of kmers [default: 31]\n"
        "  -c <i32>    Candidate set limit [default: 1]\n"
        "  -s <i32>    Size of min hash [default: 1000]\n"
        "  -d <path>   Destination directory for sketch(s)\n"
        "  -D <file>   Write to database file\n"
        "  -j          Write sketch(s) to JSON as well\n"
        "  -J          Only write sketch(s) to JSON\n"
        "  -M          Megasketch\n"
        "  -O          Same as -J\n"
        "  -t          Write to txt file\n"
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

    MinHash::set_k(31);
    MinHash::set_c(1);
    MinHash::set_s(1000);
    MinHash::set_seed(0);
    std::string fnames = "";
    std::string database_name = "";

    int option;
    while ((option = getopt(argc, argv, "f:k:c:s:d:D:o:hjMOt")) != -1)
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
            case 'D':
                database_name = optarg;
            case 'j' :
                Sketch::write_json = true;
                break;
            case 'M':
                MinHash::set_s(0);
                break;
            case 'O' :
                Sketch::write_only_json = true;
                break;
            case 'f':
                fnames = optarg;
                break;
            case 't':
                Sketch::write_txt = true;
                break;
            case 'h' :
                print_usage();
                exit(0);
        }
    }

    if (fnames != "")
    {
        std::fstream fin;
        fin.open(fnames, std::ios::in);
        if (fin.is_open())
        {
            std::string fname;
            while (getline(fin, fname))
                Sketch{fname};
        } 
    }
    else
    {
        for (; optind < argc; optind++)
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

    return 0;
}
