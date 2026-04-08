#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>

using namespace std;

// 儲存棋盤狀態與棋子高度的 9x9 陣列
int board[9][9];
int layer[9][9];
int my_color; // 0 代表白棋 (White), 1 代表黑棋 (Black)

// 判斷該位置的棋子是否為己方
bool is_mine(int p) {
    if (p <= 0) return false;
    return my_color == 0 ? (p >= 1 && p <= 3) : (p >= 4 && p <= 6);
}

// 判斷該位置的棋子是否為敵方
bool is_enemy(int p) {
    if (p <= 0) return false;
    return my_color == 0 ? (p >= 4 && p <= 6) : (p >= 1 && p <= 3);
}

// 定義移動結構與輸出格式
struct Move {
    char action;
    int sx, sy, ex, ey;
    void print() {
        cout << action << "," << sx << "," << sy << "," << ex << "," << ey << endl;
    }
};

// Hex Grid 的六個移動方向 (Axial Coordinates)
int dx[] = {1, -1, 0, 0, 1, -1};
int dy[] = {0, 0, 1, -1, -1, 1};

// 取得所有合法的吃子動作 (Eat)
vector<Move> get_eat_moves() {
    vector<Move> moves;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (is_mine(board[i][j])) {
                int sx = j - 4; // 將陣列索引轉換為題目要求的實際座標 (-4 ~ 4)
                int sy = i - 4;
                for (int d = 0; d < 6; d++) {
                    int cx = sx + dx[d];
                    int cy = sy + dy[d];
                    while (cx >= -4 && cx <= 4 && cy >= -4 && cy <= 4) {
                        if (cx == 0 && cy == 0) break; // 中心點不能跨越
                        
                        int p = board[cy + 4][cx + 4];
                        if (p == -1) break; // 超出棋盤邊界
                        
                        if (p != 0) { // 遇到棋子
                            if (is_enemy(p)) {
                                // 高度必須大於等於敵方才能吃子
                                if (layer[sy + 4][sx + 4] >= layer[cy + 4][cx + 4]) {
                                    moves.push_back({'E', sx, sy, cx, cy});
                                }
                            }
                            break; // 不能跳過任何棋子
                        }
                        cx += dx[d];
                        cy += dy[d];
                    }
                }
            }
        }
    }
    return moves;
}

// 取得所有合法的併子動作 (Stack)
vector<Move> get_stack_moves() {
    vector<Move> moves;
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (is_mine(board[i][j])) {
                int sx = j - 4;
                int sy = i - 4;
                for (int d = 0; d < 6; d++) {
                    int cx = sx + dx[d];
                    int cy = sy + dy[d];
                    while (cx >= -4 && cx <= 4 && cy >= -4 && cy <= 4) {
                        if (cx == 0 && cy == 0) break; 
                        
                        int p = board[cy + 4][cx + 4];
                        if (p == -1) break; 
                        
                        if (p != 0) { 
                            if (is_mine(p)) {
                                moves.push_back({'S', sx, sy, cx, cy});
                            }
                            break; 
                        }
                        cx += dx[d];
                        cy += dy[d];
                    }
                }
            }
        }
    }
    return moves;
}

// 評估棋子種類價值 (Tzaar > Tzarra > Totts)
int get_piece_type(int p) {
    if (p == 1 || p == 4) return 3; // 沙皇 (最高價值)
    if (p == 2 || p == 5) return 2; // 沙后
    if (p == 3 || p == 6) return 1; // 沙兵
    return 0;
}

// 簡單啟發式評估函式，尋找最佳步
int evaluate_move(Move m) {
    if (m.action == 'P') return 0;
    if (m.action == 'E') {
        int target = board[m.ey + 4][m.ex + 4];
        return 10 * get_piece_type(target); // 優先吃掉對方重要的棋子
    }
    if (m.action == 'S') {
        int self_piece = board[m.sy + 4][m.sx + 4];
        return 5 * get_piece_type(self_piece); // 優先保護自己重要的棋子
    }
    return 0;
}

// 在內部狀態模擬棋子移動
void apply_move(Move m) {
    if (m.action == 'P') return;
    
    // 目標地的棋子種類會變成來源地的棋子種類 (疊加後由上方決定)
    board[m.ey + 4][m.ex + 4] = board[m.sy + 4][m.sx + 4];
    board[m.sy + 4][m.sx + 4] = 0;
    
    if (m.action == 'S') {
        layer[m.ey + 4][m.ex + 4] += layer[m.sy + 4][m.sx + 4];
    }
    if (m.action == 'E') {
        // 吃子時，來源層數直接取代目標格層數
        layer[m.ey + 4][m.ex + 4] = layer[m.sy + 4][m.sx + 4];
    }
    layer[m.sy + 4][m.sx + 4] = 0;
}

int main(int argc, char* argv[]) {
    // 參數檢查
    if (argc < 6) {
        cerr << "Usage: ./bot.out [Color] [Round] [board] [chessLayer] [stepHistory]" << endl;
        return 1;
    }
    
    // 初始化隨機亂數種子，避免每局動作過於單調
    srand(time(NULL));

    string colorStr = argv[1];
    int round = stoi(argv[2]);
    string boardFile = argv[3];
    string layerFile = argv[4];
    // argv[5] 為 stepHistory，簡易策略目前不需要用到所以不讀取
    
    my_color = (colorStr == "White") ? 0 : 1;
    
    // 讀取盤面分布
    ifstream bf(boardFile);
    if (bf.is_open()) {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                bf >> board[i][j];
            }
        }
        bf.close();
    }
    
    // 讀取高度分布
    ifstream lf(layerFile);
    if (lf.is_open()) {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                lf >> layer[i][j];
            }
        }
        lf.close();
    }
    
    // ====== 第一步：強制吃子 (Required) ======
    vector<Move> eats = get_eat_moves();
    if (eats.empty()) {
        // 若無法吃子代表已無路可走，但仍須輸出格式避免程式崩潰
        cout << "E,0,0,0,0\nP,0,0,0,0\n";
        return 0;
    }
    
    Move best_step1 = eats[0];
    int best_score1 = -1;
    for (Move m : eats) {
        // 加入亂數抖動，讓相同分數時也有多樣的選步
        int score = evaluate_move(m) + (rand() % 5);
        if (score > best_score1) {
            best_score1 = score;
            best_step1 = m;
        }
    }
    
    // 輸出第一步
    best_step1.print();
    
    // 第一回合白棋只能做一動，第二步必定為 Pass
    if (round == 1 && my_color == 0) {
        cout << "P,0,0,0,0\n";
        return 0;
    }
    
    // 在內部陣列模擬第一步的結果，確保第二步的合法性
    apply_move(best_step1);
    
    // ====== 第二步：三選一 (Optional Choice) ======
    vector<Move> step2_moves;
    vector<Move> eats2 = get_eat_moves();
    vector<Move> stacks2 = get_stack_moves();
    
    // 將所有可行的「吃子」和「併子」存入
    step2_moves.insert(step2_moves.end(), eats2.begin(), eats2.end());
    step2_moves.insert(step2_moves.end(), stacks2.begin(), stacks2.end());
    
    Move best_step2 = {'P', 0, 0, 0, 0};
    int best_score2 = 0; // Pass 的基礎分數為 0
    
    for (Move m : step2_moves) {
        int score = evaluate_move(m) + (rand() % 5);
        if (score > best_score2) {
            best_score2 = score;
            best_step2 = m;
        }
    }
    
    // 輸出第二步
    best_step2.print();
    
    return 0;
}
