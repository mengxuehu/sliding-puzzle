#include <chrono>
#include <climits>
#include <cmath>
#include <set>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>

using std::cout;
using std::endl;

#define N 16


class Solver {
private:
    std::map<int, std::vector<int>> next_step[N];
    std::vector<int> pos_blank;
    std::vector<int> movement;
    int node[N];
    int depth;
    int threshold;
    const static int FOUND = 0;

    long count;  // node generation
    const int sqrt_n = (int) std::sqrt(N);

    int dist[N][N];  // Manhattan distance

public:
    Solver() {
        const int num_direction = 4;
        int move[num_direction] = {-sqrt_n, sqrt_n, -1, 1};  // up, down, left, right
        int bar[] = {0, sqrt_n - 1, 0, sqrt_n - 1};
        for (int i = 0; i < sqrt_n; ++i) {
            for (int j = 0; j < sqrt_n; ++j) {
                int foo[] = {i, i, j, j};
                std::vector<int> target;
                for (int k = 0; k < num_direction; ++k) {
                    if (foo[k] != bar[k]) {
                        target.push_back(move[k]);
                    }
                }

                std::map<int, std::vector<int>> tmp_next_step;
                for (int l : target) {
                    std::vector<int> tmp = target;
                    for (auto it = tmp.begin(); it != tmp.end(); ++it) {
                        if (*it == l) {
                            tmp.erase(it);
                            break;
                        }
                    }
                    tmp_next_step[-l] = tmp;
                }
                tmp_next_step[0] = target;
                next_step[i * sqrt_n + j] = tmp_next_step;
            }
        }
    }

    bool is_solvable(int *init) {
        int num_inversion = 0;
        for (int i = 0; i < N; ++i) {
            for (int j = i + 1; j < N; ++j) {
                if (init[i] == 0 || init[j] == 0) {
                    continue;
                } else if (init[i] > init[j]) {
                    num_inversion += 1;
                }
            }
        }
        return (num_inversion & 0x1) == 0;
    }

    void solve(int *init) {
        init_manhattan();
        threshold = heuristic_manhattan(init);

        /* initialize root node */
        std::copy(init, init + N, node);
        for (int i = 0; i < N; ++i) {
            if (node[i] == 0) {
                pos_blank.push_back(i);
                break;
            }
        }
        movement.push_back(0);
        depth = 0;
        count = 1;

        while (threshold != FOUND) {
            pos_blank.resize((unsigned) threshold);
            movement.resize((unsigned) threshold);
            threshold = search();
            cout <<"threshold: " << threshold << " count: " << count << endl;
        }

        track_path(init);
        cout << "depth: " << depth << " count: " << count << endl;
    }

private:
    int heuristic_hamming(int *node_) {
        int result = 0;
        for (int i = 1; i < N; ++i) {
            if (i != node_[i]) {
                result += 1;
            }
        }
        return result;
    }

    int heuristic_manhattan(int *node_) {
        int result = 0;
        for (int i = 0; i < N; ++i) {
            if (node_[i] != 0) {
                result += dist[node_[i]][i];
            }
        }
        return result;
    }

    void init_manhattan() {
        for (int i = 0; i < N; ++i) {
            dist[0][N] = 0;
        }
        for (int i = 1; i < N; ++i) {
            for (int j = 0; j < N; ++j) {
                dist[i][j] = std::abs(i / sqrt_n - j / sqrt_n) + std::abs(i % sqrt_n - j % sqrt_n);
            }
        }
    }

    void generate_database() {

    }

    void track_path(int *init) {
        int tmp[N];
        std::copy(init, init + N, tmp);
        for (auto d = 0;; ++d) {
            for (auto i : tmp) {
                cout << i << " ";
            }
            cout << endl;

            if (d == depth) {
                return;
            }

            int blank_source = pos_blank[d], blank_target = pos_blank[d + 1];
            tmp[blank_source] = tmp[blank_target];
            tmp[blank_target] = 0;
        }
    }

    int search() {
        int h = heuristic_manhattan(node);
        if (h == 0) {
            return FOUND;
        }
        int f = depth + h;
        if (f > threshold) {
            return f;
        }
        int p = pos_blank[depth];
        int m = movement[depth];
        int min_ = INT_MAX;
        for (auto i : next_step[p][m]) {
            count += 1;

            node[p] = node[p + i];
            node[p + i] = 0;

            depth += 1;
            pos_blank[depth] = p + i;
            movement[depth] = i;

            f = search();
            if (f == FOUND) {
                return FOUND;
            }
            if (f < min_) {
                min_ = f;
            }

            node[p + i] = node[p];
            node[p] = 0;
            depth -= 1;
        }
        return min_;
    }
};

int main() {
    Solver s;
    int init[N];
    std::iota(init, init + N, 0);
//    int init[N] = {8, 7, 6, 0, 4, 1, 2, 5, 3};
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
