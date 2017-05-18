#include <climits>
#include <string>
#include <cmath>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <chrono>

#include "solver.h"

using std::cout;
using std::endl;


Solver::~Solver() {
    delete[](row_linear_conflict_and_manhattan);
    delete[](col_linear_conflict);
}

Solver::Solver(bool serialize_data) {
    row_linear_conflict_and_manhattan = new int[db_size];
    col_linear_conflict = new int[db_size];
    init_next_step();
    init_manhattan();
    if (serialize_data) {
        save_load_data("data-" + std::to_string(N));
    } else {
        init_heuristic_data();
    }
}

bool Solver::is_solvable(int *init) {
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

void Solver::solve(int *init) {
    auto start_time = std::chrono::system_clock::now();
    pos_blank.clear();
    movement.clear();
    threshold = heuristic(init);

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
        cout << "threshold: " << threshold << " count: " << count << endl;
    }

    cout << "run time: " << (std::chrono::system_clock::now() - start_time).count() << endl;
    track_path(init);
    cout << "depth: " << depth << " count: " << count << endl << endl;
}


int Solver::heuristic_hamming(int *node_) {
    int result = 0;
    for (int i = 1; i < N; ++i) {
        if (i != node_[i]) {
            result += 1;
        }
    }
    return result;
}

int Solver::heuristic_manhattan(int *node_) {
    int result = 0;
    for (int i = 0; i < N; ++i) {
        if (node_[i] != 0) {
            result += dist[node_[i]][i];
        }
    }
    return result;
}

/* Manhattan distance and linear conflict */
int Solver::heuristic(int *node_) {
    int result = 0;
    for (int i = 0; i < sqrt_n; ++i) {
        int idx1 = node_[i * sqrt_n], idx2 = node_[i];
        for (int j = 1; j < sqrt_n; ++j) {
            idx1 = idx1 * N + node_[i * sqrt_n + j];
            idx2 = idx2 * N + node_[j * sqrt_n + i];
        }
        result += row_linear_conflict_and_manhattan[i * db_size_line + idx1]
                  + col_linear_conflict[i * db_size_line + idx2];
    }
    return result;
}

void Solver::init_next_step() {
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

            std::unordered_map<int, std::vector<int>> tmp_next_step;
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

void Solver::init_manhattan() {
    for (int i = 1; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            dist[i][j] = std::abs(i / sqrt_n - j / sqrt_n) + std::abs(i % sqrt_n - j % sqrt_n);
        }
    }
}


void Solver::save_load_data(std::string file_name) {
    std::ifstream in(file_name, std::ios::in);
    if (!in) {
        init_heuristic_data();
        std::ofstream out(file_name, std::ios::out);

        for (int i = 0; i < db_size; ++i) {
            out << row_linear_conflict_and_manhattan[i];
            out << " ";
        }


        for (int i = 0; i < db_size; ++i) {
            out << col_linear_conflict[i];
            out << " ";
        }

        out.close();
    } else {
        for (int i = 0; i < db_size; ++i) {
            in >> row_linear_conflict_and_manhattan[i];
        }

        for (int i = 0; i < db_size; ++i) {
            in >> col_linear_conflict[i];
        }

        in.close();
    }
}


void Solver::init_heuristic_data() {
    std::string rest;
    for (int i = 0; i < N; ++i) {
        rest += (char) i;
    }
    partial_permutation("", rest, sqrt_n);
}


void Solver::partial_permutation(std::string so_far, std::string rest, int n) {
    if (n == 0) {
        this->init_database_per_partial_permutation(so_far);
    } else {
        for (size_t i = 0; i < rest.length(); i++) {
            partial_permutation(so_far + rest[i], rest.substr(0, i) + rest.substr(i + 1, rest.length()), n - 1);
        }
    }
}

void Solver::init_database_per_partial_permutation(std::string &so_far) {
    int line_count = 0;
    std::map<int, std::vector<int>> row, col;
    int tiles[sqrt_n];
    for (int i = 0; i < sqrt_n; ++i) {
        tiles[i] = (int) so_far[i];
        line_count = line_count * N + tiles[i];
        if (tiles[i] != 0) {
            int r = tiles[i] / sqrt_n, c = tiles[i] % sqrt_n;

            if (row.end() == row.find(r)) {
                std::vector<int> tmp(1, tiles[i]);
                row[r] = tmp;
            } else {
                row[r].push_back(tiles[i]);
            }

            if (col.end() == col.find(c)) {
                std::vector<int> tmp(1, tiles[i]);
                col[c] = tmp;
            } else {
                col[c].push_back(tiles[i]);
            }

        }
    }

    // manhattan distance
    for (int i = 0; i < sqrt_n; ++i) {
        int d = 0, idx = i * sqrt_n;
        for (int j = 0; j < sqrt_n; ++j) {
            if (tiles[j] != 0) {
                d += dist[tiles[j]][idx + j];
            }
        }
        row_linear_conflict_and_manhattan[i * db_size_line + line_count] += d;
    }

    // row linear conflict
    for (auto r : row) {
        std::vector<int> tmp_max(r.second.size(), 0);

        for (int i = 0; i < r.second.size(); ++i) {
            for (int j = i + 1; j < r.second.size(); ++j) {
                if (r.second[i] > r.second[j]) {
                    ++tmp_max[i];
                    ++tmp_max[j];
                }
            }
        }
        row_linear_conflict_and_manhattan[r.first * db_size_line + line_count] +=
                (*std::max_element(tmp_max.begin(), tmp_max.end()) << 1);
    }

    // column linear conflict
    for (auto c : col) {
        std::vector<int> tmp_max(c.second.size(), 0);

        for (int i = 0; i < c.second.size(); ++i) {
            for (int j = i + 1; j < c.second.size(); ++j) {
                if (c.second[i] > c.second[j]) {
                    ++tmp_max[i];
                    ++tmp_max[j];
                }
            }
        }
        col_linear_conflict[c.first * db_size_line + line_count] +=
                (*std::max_element(tmp_max.begin(), tmp_max.end()) << 1);
    }
}


void Solver::track_path(int *init) {
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


int Solver::search() {
    int h = heuristic(node);
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
