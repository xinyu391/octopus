#include "pool.h"
#include <iostream>
#include <tuple>

int func(void* p) {
    long i = (long)p;
    using namespace std::chrono_literals;
    int c = 0;
    for (int n = 0; n < 2000; n++) {
        std::this_thread::sleep_for(1ms);
        c += n;
    }
    std::cout << " thread " << i << " " << std::this_thread::get_id() << std::endl;
    // printf("working task %ld  %d\n", i, c);
    return 0;
}

int func1(void* p, int a) {
    long i = (long)p;
    using namespace std::chrono_literals;
    int c = 0;
    for (int n = 0; n < 2000; n++) {
        std::this_thread::sleep_for(1ms);
        c += n;
    }
    std::cout << " threadxx " << i << " " << a << " " << std::this_thread::get_id() << std::endl;
    // printf("working taskxx %ld  %d\n", i, c);
    return 0;
}
class Mat {
public:
    Mat() {
        n = 0;
        printf("nnnnnnnnnnnnnnnnnnnnnnnnnn\n");
    };
    int n;
    Mat& operator=(const Mat& m) {
        printf("================\n");
        if (this == &m) {
            return *this;
        }
        n = m.n;
        return *this;
    };
};
int main(int argc, char* argv[]) {
    // long a = 10;
    // func2(func, (void*)a);

    // int ii = 10;
    // func2(func1, ii);

    // return 0;
    octopus::Pool pool(4);
    std::vector<Mat> results(5);
    std::vector<Mat> output(5);
    std::vector<std::future<void>> tids;
    printf("pool ready to work\n");
    for (int i = 0; i < 5; i++) {
        long ii = i;
        printf("mat %d  %p\n", results[i].n, &results[i]);
        
        output[i].n = 10;
        std::future<void> tid = pool.enqueue([i,&results](int n, Mat& mat) {printf("xxxxxxxxxx %d %p\n",i, &results[n]);results[n].n = n; mat.n = n; }, i, output[i]);
        tids.push_back(std::move(tid));
        printf("pool add %ld \n", ii);
    }
    printf("pool wait 1\n");
    for (int i = 0; i < tids.size(); i++) {
        tids[i].get();
        printf("mat %d - %d  %p\n", output[i].n,results[i].n, &results[i]);
    }
    // pool.wait(1);

    printf("pool ready to destroy\n");
    printf("pool finish\n");

    return 0;
}