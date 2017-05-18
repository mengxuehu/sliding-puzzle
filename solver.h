
#ifndef N_PUZZLE_SOLVER_H
#define N_PUZZLE_SOLVER_H

#include <cmath>
#include <unordered_map>
#include <vector>
#include <string>


#define N 9


class Solver {
private:
    const static int sqrt_n = (int) std::sqrt(N);
    const static int db_size_line = (int) pow((double) N, (double) sqrt_n);
    const static int db_size = sqrt_n * db_size_line;//2*1046993+1;//


    std::unordered_map<int, std::vector<int>> next_step[N];
    int dist[N][N] = {};  // Manhattan distance

    int *row_linear_conflict_and_manhattan;//[db_size];// = {};
    int *col_linear_conflict;//[db_size];// = {};

private:
    const static int FOUND = 0;

    long count;  // node generation
    int depth;
    int threshold;

    int node[N];

    std::vector<int> pos_blank;
    std::vector<int> movement;

public:
    ~Solver();

    Solver(bool serialize_data = false);

    bool is_solvable(int *init);

    void solve(int *init);

private:
    int heuristic_hamming(int *node_);

    int heuristic_manhattan(int *node_);

    /* Manhattan distance and linear conflict */
    int heuristic(int *node_);

    void init_next_step();

    void init_manhattan();

    void save_load_data(std::string file_name);

    void init_heuristic_data();

    void partial_permutation(std::string so_far, std::string rest, int n);

    void init_database_per_partial_permutation(std::string &rest);

    void track_path(int *init);

    int search();
};


#endif //N_PUZZLE_SOLVER_H
