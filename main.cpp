#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

using namespace std;

int board[9][9], layer[9][9], my_color;

// 判斷棋子陣營 (白棋:1~3, 黑棋:4~6)
bool is_mine(int p) { return p > 0 && (my_color == 0 ? p <= 3 : p >= 4); }
bool is_enemy(int p) { return p > 0 && (my_color == 0 ? p >= 4 : p <= 3); }

// 評估棋子價值：沙皇(Tzaar)最重要，給 100 分！
int evaluate(int p) {
    if (p == 1 || p == 4) return 100; // 沙皇 Tzaar
    if (p == 2 || p == 5) return 10;  // 沙后 Tzarra
    if (p == 3 || p == 6) return 1;   // 沙兵 Totts
    return 0;
}

// 動作結構，輸出時自動把 0~8 的內部座標轉回題目的 -4~4
struct Move {
    char action;
    int x1, y1, x2, y2;
    void print() {
        cout << action << "," << x1 - 4 << "," << y1 - 4 << "," << x2 - 4 << "," << y2 - 4 << endl;
    }
};

// Hex Grid 六個移動方向
int dx[] = {1, -1, 0, 0, 1, -1};
int dy[] = {0, 0, 1, -1, -1, 1};

// 模擬移動更新內部盤面
void apply_move(Move m) {
    if (m.action == 'P') return;
    board[m.y2][m.x2] = board[m.y1][m.x1];
    board[m.y1][m.x1] = 0;
    layer[m.y2][m.x2] = (m.action == 'S') ? layer[m.y2][m.x2] + layer[m.y1][m.x1] : layer[m.y1][m.x1];
    layer[m.y1][m.x1] = 0;
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    my_color = (string(argv[1]) == "White") ? 0 : 1;
    int round = stoi(argv[2]);
    
    // 讀取盤面與高度
    ifstream bf(argv[3]), lf(argv[4]);
    for (int y = 0; y < 9; y++) for (int x = 0; x < 9; x++) { bf >> board[y][x]; lf >> layer[y][x]; }

    // ====== 第一步：尋找最高分的「吃子」 ======
    Move best_step1 = {'E', 4, 4, 4, 4}; // 預設空步
    int max_score1 = -1;

    for (int y = 0; y < 9; y++) for (int x = 0; x < 9; x++) {
        if (is_mine(board[y][x])) {
            for (int d = 0; d < 6; d++) {
                int cx = x, cy = y;
                while (true) {
                    cx += dx[d]; cy += dy[d];
                    // 撞牆或經過中心點(4,4)則停止探索該方向
                    if (cx < 0 || cx > 8 || cy < 0 || cy > 8 || board[cy][cx] == -1 || (cx == 4 && cy == 4)) break;
                    
                    int target = board[cy][cx];
                    if (target != 0) { // 遇到第一顆棋子
                        if (is_enemy(target) && layer[y][x] >= layer[cy][cx]) {
                            int score = evaluate(target) + (rand() % 3); // 價值 + 微小亂數抖動
                            if (score > max_score1) { max_score1 = score; best_step1 = {'E', x, y, cx, cy}; }
                        }
                        break; // 不能跳過棋子
                    }
                }
            }
        }
    }

    if (max_score1 == -1) { cout << "E,0,0,0,0\nP,0,0,0,0\n"; return 0; } // 沒得吃就等死
    best_step1.print();
    if (round == 1 && my_color == 0) { cout << "P,0,0,0,0\n"; return 0; } // 白棋第一回合強制結束
    apply_move(best_step1); // 模擬第一步，改變盤面以計算第二步

    // ====== 第二步：尋找最高分的「吃子」或「併子」(也可以 Pass) ======
    Move best_step2 = {'P', 4, 4, 4, 4}; // 座標 4-4=0，印出剛好是 P,0,0,0,0
    int max_score2 = 0; // Pass的基礎分是 0，有更好的才取代

    for (int y = 0; y < 9; y++) for (int x = 0; x < 9; x++) {
        if (is_mine(board[y][x])) {
            for (int d = 0; d < 6; d++) {
                int cx = x, cy = y;
                while (true) {
                    cx += dx[d]; cy += dy[d];
                    if (cx < 0 || cx > 8 || cy < 0 || cy > 8 || board[cy][cx] == -1 || (cx == 4 && cy == 4)) break;
                    
                    int target = board[cy][cx];
                    if (target != 0) {
                        int score = evaluate(target) + (rand() % 3);
                        if (is_enemy(target) && layer[y][x] >= layer[cy][cx] && score > max_score2) {
                            max_score2 = score; best_step2 = {'E', x, y, cx, cy}; // 吃子
                        } 
                        else if (is_mine(target) && score > max_score2) {
                            max_score2 = score; best_step2 = {'S', x, y, cx, cy}; // 併子保護自己
                        }
                        break;
                    }
                }
            }
        }
    }
    
    best_step2.print();
    return 0;
}
