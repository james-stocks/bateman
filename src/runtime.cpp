#include <cstdio>
#include <cstdlib>

extern "C" void bateman_throw(const char* message) {
    fprintf(stderr, "Hey, Paul! Exception: %s\n", message);
    exit(1);
}

extern "C" void bateman_print(int value) {
    printf("%d\n", value);
}

extern "C" int bateman_read() {
    int value;
    scanf("%d", &value);
    return value;
}