#include "solver.h"

#include <iostream>
#include <numeric>
#include <chrono>
#include <algorithm>

using std::cout;
using std::endl;

int main() {
    Solver s(false);
    int init[N];
    std::iota(init, init + N, 0);
    bool flag = true;

    while (flag) {
        long seed = std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(init, init + N, std::default_random_engine(seed));
        for (int i : init) {
            cout << i << " ";
        }
        cout << endl;
        if (s.is_solvable(init)) {
            flag = false;
            s.solve(init);
        } else {
            cout << "unsolvable" << endl;
        }
    }

    return 0;
}