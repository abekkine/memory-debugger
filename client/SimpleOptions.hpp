#ifndef SIMPLE_OPTIONS_HPP_
#define SIMPLE_OPTIONS_HPP_

#include <string>
#include <vector>

class SimpleOptions {
public:
    explicit SimpleOptions(int argc, char **argv) {
        for (int i=1; i<argc; ++i) {
            options_.push_back(std::string(argv[i]));
        }
    }
    ~SimpleOptions() {}
    bool hasOption(std::string opt) {
        bool found = false;
        for (auto & option : options_) {
            if (option == opt) {
                found = true;
                break;
            }
        }
        return found;
    }

private:
    std::vector<std::string> options_;
};

#endif // SIMPLE_OPTIONS_HPP_
