#include <GL/gl.h>
#include <GL/glut.h>

#include <vector>

#include "SocketInterface.h"

#include "SimpleOptions.hpp"
#include "UdpSocket.hpp"
#include "TcpServerSocket.hpp"

SocketInterface * debugSocket = 0;

static bool debug_alloc = false;

#pragma pack(push, 1)
struct MemData {
    uint16_t sequence;
    char type;
    unsigned int address;
    unsigned int size;
    char label[32];
};

struct MemoryBlock {
    MemoryBlock(const std::string & label, unsigned int s, unsigned int e)
        : label(label)
        , start(s)
        , end(e)
        , count(1)
    {}
    std::string label;
    unsigned int start;
    unsigned int end;
    unsigned int dStart;
    unsigned int dEnd;
    int count;
};
#pragma pack(pop)

const int WINDOW_MARGIN = 50;
const int MEMORY_DISPLAY_WIDTH = 200;
const int MEMORY_DISPLAY_HEIGHT = 1600;
const int WINDOW_WIDTH = MEMORY_DISPLAY_WIDTH + (WINDOW_MARGIN << 1);
const int WINDOW_HEIGHT = MEMORY_DISPLAY_HEIGHT + (WINDOW_MARGIN << 1);

unsigned int memory_lo_ = 0xffffffff;
unsigned int memory_hi_ = 0x0;
unsigned int memory_span_ = 0;
std::vector<MemoryBlock *> memory_map_;
unsigned int gap_lo_ = 0x0;
unsigned int gap_hi_ = 0x0;

uint16_t last_sequence_ = 0;

void convert_to_display(MemoryBlock * mb) {
    mb->dStart = MEMORY_DISPLAY_HEIGHT * (uint64_t)(mb->start - memory_lo_) / memory_span_;
    mb->dEnd = MEMORY_DISPLAY_HEIGHT * (uint64_t)(mb->end - memory_lo_) / memory_span_;
}

void update_memory_span(const unsigned int start, const unsigned int end) {
    if (start < memory_lo_) {
        memory_lo_ = start;
    }
    if (end > memory_hi_) {
        memory_hi_ = end;
    }
    memory_span_ = memory_hi_ - memory_lo_;
}

void update_display_positions() {
    for (auto mb : memory_map_) {
        convert_to_display(mb);
    }
}

void reset_memory_map() {
    memory_lo_ = 0xffffffff;
    memory_hi_ = 0;
    memory_span_ = 0;
    for (auto mb : memory_map_) {
        delete mb;
    }
    memory_map_.clear();
}

MemoryBlock * find_memory_block(const unsigned int start, const unsigned int end) {
    MemoryBlock * block = 0;
    for (auto mb : memory_map_) {
        if (mb->start == start && mb->end == end) {
            block = mb;
            break;
        }
    }
    return block;
}

void debug_dump(const char *label, MemoryBlock * mb) {
    convert_to_display(mb);
    printf(
        "%3s R[%08x..%08x] C(%d) S(%d) D(%d..%d) MS(%d)\n"
        , label
        , mb->start
        , mb->end
        , mb->count
        , mb->end-mb->start+1
        , mb->dStart
        , mb->dEnd
        , memory_span_
    );
}

void update_memory_map(MemData & data) {
    // Update limits
    const unsigned int mem_start = data.address;
    const unsigned int mem_end = data.address + data.size - 1;
    const std::string mem_label = std::string(data.label);

    if (data.type != 'c') {
        update_memory_span(mem_start, mem_end);
        update_display_positions();
    }

    MemoryBlock * block = 0;
    block = find_memory_block(mem_start, mem_end);
    if (data.type == 'a') {
        if (block != 0) {
            ++(block->count);
            if (debug_alloc) {
                debug_dump("inc", block);
            }
        }
        else {
            block = new MemoryBlock(mem_label, mem_start, mem_end);
            memory_map_.push_back(block);
            if (debug_alloc) {
                debug_dump("add", block);
            }
        }
    }
    else if (data.type == 'r') {
        if (block != 0) {
            --(block->count);
            if (debug_alloc) {
                debug_dump("dec", block);
            }
            if (block->count < 0 && debug_alloc) {
                debug_dump(mem_label.c_str(), block);
            }
        }
        else {
            block = new MemoryBlock(mem_label, mem_start, mem_end);
            block->count = -1;
            memory_map_.push_back(block);
            if (debug_alloc) {
                debug_dump("rem", block);
            }
            debug_dump(mem_label.c_str(), block);
        }
    }
    else if (data.type == 'c') {
        reset_memory_map();
    }
}

void render_memory_map() {

    for (auto mb : memory_map_) {
        glLoadIdentity();
        glTranslated(WINDOW_MARGIN, WINDOW_MARGIN, 0.0);
        if (mb->count < 0) {
            glColor4f(1.0, 0.0, 0.0, 0.2);
        }
        else if (mb->count > 0) {
            glColor4f(0.0, 1.0, 0.0, 0.2);
        }
        else {
            glColor4f(0.5, 0.5, 0.5, 0.2);
        }
        if (mb->dStart == mb->dEnd) {
            glBegin(GL_LINES);
            glVertex2i(0, mb->dStart);
            glVertex2i(MEMORY_DISPLAY_WIDTH, mb->dStart);
            glEnd();
        }
        else {
            glBegin(GL_QUADS);
            glVertex2i(0, mb->dStart);
            glVertex2i(MEMORY_DISPLAY_WIDTH, mb->dStart);
            glVertex2i(MEMORY_DISPLAY_WIDTH, mb->dEnd);
            glVertex2i(0, mb->dEnd);
            glEnd();
        }
    }
}

void render_memory_span() {
    glLoadIdentity();
    glTranslated(WINDOW_MARGIN, WINDOW_MARGIN, 0.0);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINE_LOOP);
    glVertex2i(-4, -4);
    glVertex2i(MEMORY_DISPLAY_WIDTH + 4, -4);
    glVertex2i(MEMORY_DISPLAY_WIDTH + 4, MEMORY_DISPLAY_HEIGHT + 4);
    glVertex2i(-4, MEMORY_DISPLAY_HEIGHT + 4);
    glEnd();
}

void render() {

    glClear(GL_COLOR_BUFFER_BIT);

    render_memory_span();
    render_memory_map();

    glutSwapBuffers();
    glutPostRedisplay();
}

void key(unsigned char key, int x, int y) {

    (void)x;
    (void)y;

    if (key == 27) {
        exit(0);
    }
}

void idle() {
    MemData data;
    try {
        if (debugSocket->Read(&data, sizeof(MemData)) > 0) {
            if (last_sequence_ + 1 != data.sequence) {
                printf("Missing package (%04x)\n", data.sequence);
            }
            last_sequence_ = data.sequence;
            // printf("%s %c\n", data.label, data.type);
            update_memory_map(data);
        }
    }
    catch (std::string & eMsg) {
        printf("Error: %s\n", eMsg.c_str());
    }
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv)
{
    SimpleOptions opts(argc, argv);
    bool use_udp = true;
    if (opts.hasOption("-d")) {
        debug_alloc = true;
    }
    if (opts.hasOption("-tcp")) {
        use_udp = false;
    }

    if (use_udp) {
        debugSocket = new UdpSocket();
    }
    else {
        debugSocket = new TcpServerSocket();
    }

    puts("Initializing socket...");
    try {
        debugSocket->Init(2401);
    }
    catch (std::string eMsg) {
        printf("ERROR: %s\n", eMsg.c_str());
        exit(1);
    }
    puts("DONE");

    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(50, 50);
    glutCreateWindow("MemoryDebuggerClient");

    glutDisplayFunc(render);
    glutKeyboardFunc(key);
    glutIdleFunc(idle);
    glutReshapeFunc(reshape);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    glutMainLoop();

    return 0;
}
