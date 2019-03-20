extern "C" {
#include "memory_debugger.h"
}

#include <iostream>

#include "SimpleOptions.hpp"

#define USE_HEAP

char input = 0;

class Test1 {
public:
    Test1() {
        std::cout << "Test1 ";
        std::cin >> input;
        memdbg_allocate("Test1", this, sizeof(*this));

        Defaults();
    }

    ~Test1() {
        std::cout << "~Test1 ";
        std::cin >> input;
        memdbg_release("Test1", this, sizeof(*this));
    }

    void Defaults() {
        a = 1.0;
        b = 2.0;
        c = 0xff;
        d = 0xffff;
        e = 0xffffffff;
    }

private:
    float a;
    double b;
    unsigned char c;
    uint16_t d;
    uint32_t e;
};


class Test2 {
private:
    enum {
        NUM_ELEMENTS = 3,
    };
public:
    Test2() {
        std::cout << "Test2 ";
        std::cin >> input;
        memdbg_allocate("Test2", this, sizeof(*this));
    }
    ~Test2() {
        std::cout << "~Test2 ";
        std::cin >> input;
        memdbg_release("Test2", this, sizeof(*this));
    }

private:
    double numbers[5];
    Test1 mList[NUM_ELEMENTS];
};

class Test3 {
public:
    Test3() {
        std::cout << "Test3 ";
        std::cin >> input;
        memdbg_allocate("Test3", this, sizeof(*this));
#ifdef USE_HEAP
        t1 = new Test1();
        t2 = new Test1[5];
        t3 = new Test2[3];
#endif
    }
    ~Test3() {
        std::cout << "~Test3 ";
        delete t1;
        delete [] t2;
        delete [] t3;
        std::cin >> input;
        memdbg_release("Test3", this, sizeof(*this));
    }
private:
#ifdef USE_HEAP
    Test1 * t1;
    Test1 * t2;
    Test2 * t3;
#else
    Test1 t1;
    Test1 t2[5];
    Test2 t3[3];
#endif

};

int main(int argc, char **argv) {

    SimpleOptions opts(argc, argv);

    bool use_simple_scenario = false;
    if (opts.hasOption("-simple")) {
        use_simple_scenario = true;
    }

    memdbg_init_tcp(2401);
    memdbg_reset();

    if (use_simple_scenario) {
        std::cin >> input;
        void * data_block = malloc(1280);
        memdbg_allocate("Data", data_block, 1280);
        std::cin >> input;
        free(data_block);
        memdbg_release("Data", data_block, 1280);
        std::cin >> input;
    }
    else {
        Test1 X;
        Test2 Y;
        Test3 Z;
    }

    std::cout << "Example ends!";

    return 0;
}
