#include <numa.h>
#include <iostream>

template <typename T>
T* alloc(size_t size, int node = -1) {
    if(~node)
        return (T*)numa_alloc_local(size * sizeof(T));
    else
        return (T*)numa_alloc_onnode(size * sizeof(T), node);
}

template <typename T>
void free(T* buf, size_t size) {
    numa_free(buf, size * sizeof(T));
}

// Example
struct Order {
    int qty;
    float px;
    Order(int q, float p)
        : qty(q)
        , px(p)
    {}
};

void test_numa_policy() {
    Order* ord = new (alloc<Order>(1, 0)) Order(1, 1.1);
    std::cout<< ord->qty<< "@" <<ord->px << std::endl;
    free<Order>(ord, 1);
}