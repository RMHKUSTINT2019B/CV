/*----- identify_cv03class.h -----*/

#ifndef HARDWARE_H
#define HARDWARE_H

#include <iostream>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <queue>
#include <stack>
#include <vector>

#define imgsize1 256
#define sqr(a) ((a)*(a))

const int space_x[9] = {0, 1, 1, 0, -1, -1, -1, 0, 1};//9 directions
const int space_y[9] = {0, 0, 1, 1, 1, 0, -1, -1, -1};
const int dirx[4] = {1, 0, -1, 0};//4 directions up,down,left,right
const int diry[4] = {0, 1, 0, -1};
const int reduce_noise_constant2 = 30;
const int block_sizeW = 3;//sizeW,L=width,height>>block_size
const int block_sizeL = 4;//
const int co_x = 1;
const int co_y = 2;//block_sizeL is 2*block_sizeW
const int block_sqrLim = 36;//shoule be adjusted along with block_size & co_x,y
const int min_dense = 2;//the min density for a block to sprawl
typedef struct {//plane vector
    int x, y;

    int basic_direction()//consistent with dirx,y
    {
        //consider line y=x y=-x and the four parts they cut out
        if (x >= y && x > -y)return 0;
        if (x > y && x <= -y)return 3;
        if (x <= y && x < -y)return 2;
        return 1;
    }

    double modulus() {
        return sqrt(sqr(x) + sqr(y));
    }

    int modulus_sqr() {
        return sqr(x) + sqr(y);
    }

    void set(int x_, int y_) {
        x = x_;
        y = y_;
    }

    void init() {
        x = y = 0;
    }
} pl_vector;

//define basic vector calculation
pl_vector make_vec(int x, int y) {
    pl_vector a;
    a.set(x, y);
    return a;
}

pl_vector operator+(pl_vector a, pl_vector b) {
    pl_vector c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    return c;
}

pl_vector operator-(pl_vector a, pl_vector b) {
    pl_vector c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    return c;
}

pl_vector operator*(int a, pl_vector b) {
    b.x *= a;
    b.y *= a;
    return b;
}

pl_vector operator*(double a, pl_vector b) {
    b.x = round(b.x * a);
    b.y = round(b.y * a);
    return b;
}

int operator*(pl_vector a, pl_vector b) {
    return a.x * b.x + a.y * b.y;
}

typedef struct {
    int lx, ly;//location bottomleft
    int dx, dy;//size
    int cx, cy;//mass centre
    int dse;//density
    pl_vector d[4];

    void init() {
        lx = ly = dx = dy = cx = cy = dse = 0;
        for (int i = 0; i < 4; i++)d[i].init();
    }
} Block;

class Img_segment7 {
public:
    void init() {
        height = width = 0;
        result = -1;
        memset(img, 0, sizeof(img));
    }

    void input_file(char *filename) {
        std::ifstream fin(filename);
        fin >> width >> height;
        int x, y;
        char ch;
        for (y = height - 1; y >= 0; y--)
            for (x = 0; x < width; x++) {
                ch = fin.get();
                while (ch == ' ' || ch == '\n' || ch == '\r')ch = fin.get();
                if (ch == '0')img[x][y] = 0;
                else img[x][y] = 1;
            }
        fin.close();
    }

    void reduce_niose() {
        reduce_noise1();
        reduce_noise2(reduce_noise_constant2);
    }

    void output_img() {
        int x, y;
        for (y = height - 1; y >= 0; y--) {
            for (x = 0; x < width; x++)
                if (img[x][y])std::cout << "*";
                else std::cout << " ";
            std::cout << "\n";
        }
    }

    void output_img2() {
        int x, y;
        for (y = height - 1; y >= 0; y--) {
            for (x = 0; x < width; x++)
                switch (img[x][y]) {
                    case 0:std::cout << " ";
                        break;
                    case 1:std::cout << "*";
                        break;//common
                    case 2:std::cout << "+";
                        break;
                    case 3:std::cout << "x";
                        break;//remark
                    case 4:std::cout << "&";
                        break;//mass centre
                }
            std::cout << "\n";
        }
    }

    void cv_identify() {
        //L,R,T,B for the range of the process in the image
        L = width - 1;
        R = 0;
        T = 0;
        B = height - 1;
        int i, j;
        //get range
        for (i = 0; i < width; i++)
            for (j = 0; j < height; j++)
                if (img[i][j]) {
                    if (i < L)L = i;
                    if (i > R)R = i;
                    if (j < B)B = j;
                    if (j > T)T = j;
                }
        if (L > R || B > T) {
            result = -1;
            return;
        }
        //Wth,Lth for the width & height of the process area
        Wth = R - L;
        Lth = T - B;
        //LBx,LBy location bottonleft
        LBx = L;
        LBy = B;
        //A Stick
        if (Lth >= 4 * Wth) {
            result = 1;
            return;
        }
        //dx,dy for the size of each block
        //divide the image into small blocks for process
        //Notice that in cv03 the dx=2*dy because the display is such size
        dx = Wth >> block_sizeW;
        dy = Lth >> block_sizeL;
        //bx,by for the size of b[][], blocks
        bx = Wth / dx + 1;
        by = Lth / dy + 1;
        //process b[][], blocks
        for (i = 0; i < bx; i++)
            for (j = 0; j < by; j++) {
                b[i][j].lx = i * dx + LBx;
                b[i][j].ly = j * dy + LBy;
                b[i][j].dx = dx;
                b[i][j].dy = dy;
                block_process(&b[i][j]);
            }
        for (i = 0; i < bx; i++)
            for (j = 0; j < by; j++) {
                block_sprawl(i, j, block_sqrLim);
            }
        measureB();
    }

    int get_result() {
        return result;
    }

    void output_result(char *filename) {
        std::ofstream fout(filename);
        fout << result << "\n";
        fout.close();
    }

private:
    int img[imgsize1][imgsize1];
    int height, width;
    int result;
    Block b[32][32];
    int bx, by;
    //local variable in cv_identify
    int L, R, T, B;
    int LBx, LBy;//bottomleft
    int Wth, Lth;//width,length
    int dx, dy;

    //end illustration
    bool in_img(int x, int y) {
        return x >= 0 && x < width && y >= 0 && y < height;
    }

    void reduce_noise1() {
        int cou, i, j, k;
        int img_t[imgsize1][imgsize1];
        memset(img_t, 0, sizeof(img_t));
        for (i = 1; i < width - 1; i++)
            for (j = 1; j < height - 1; j++) {
                cou = 0;
                img_t[i][j] = img[i][j];
                for (k = 1; k < 9; k++)if (img[i + space_x[k]][j + space_y[k]])++cou;
                if (img[i][j] && cou < 4)img_t[i][j] = 0;
                if (!img[i][j] && cou > 5)img_t[i][j] = 1;
            }
        for (i = 0; i < width; i++)
            for (j = 0; j < height; j++)img[i][j] = img_t[i][j];
    }

    void reduce_noise2(int set_max) {
        int i, j, k;
        bool vis[imgsize1][imgsize1];
        memset(vis, 0, sizeof(vis));
        pl_vector p;
        pl_vector u;
        pl_vector v;
        std::queue<pl_vector> q;
        std::stack<pl_vector> s;
        int sum;
        for (i = 0; i < width; i++)
            for (j = 0; j < height; j++)
                if (img[i][j] && !vis[i][j]) {
                    sum = 0;
                    while (!q.empty())q.pop();
                    while (!s.empty())s.pop();
                    p.x = i;
                    p.y = j;
                    q.push(p);
                    s.push(p);
                    while (!q.empty()) {
                        u = q.front();
                        q.pop();
                        for (k = 1; k < 9; k++) {
                            v.x = u.x + space_x[k];
                            v.y = u.y + space_y[k];
                            if (in_img(v.x, v.y) && img[v.x][v.y] && !vis[v.x][v.y]) {
                                vis[v.x][v.y] = true;
                                ++sum;
                                q.push(v);
                                s.push(v);
                            }
                        }
                    }
                    if (sum < set_max) {
                        while (!s.empty()) {
                            u = s.top();
                            img[u.x][u.y] = 0;
                            s.pop();
                        }
                    }
                }
    }

    int sqr_dis(pl_vector a, pl_vector b) {
        return sqr(a.x - b.x) + sqr(a.y - b.y);
    }

    //measure the actual distance that function "sprawl" travels when there is coefficient co_x,y
    int sqr_block_dis(pl_vector a, pl_vector b, int cox, int coy) {
        return sqr(coy) * sqr(a.x - b.x) + sqr(cox) * sqr(a.y - b.y);
    }

    //get basic info of the block
    void block_process(Block *b) {
        int i, j;
        b->dse = 0;
        pl_vector v;
        v.init();
        for (i = b->lx; i < b->lx + b->dx; i++)
            for (j = b->ly; j < b->ly + b->dy; j++)
                if (in_img(i, j) && img[i][j]) {
                    ++b->dse;
                    v.x += i - b->lx;
                    v.y += j - b->ly;
                }
        if (b->dse) {
            b->cx = b->lx + v.x / b->dse;
            b->cy = b->ly + v.y / b->dse;
        }
    }

    //b[i][j] is in process area
    bool block_exist(int i, int j) {
        return i >= 0 && i < bx && j >= 0 && j < by;
    }

    //get the vectors, say, direction of a block considering its adjacent blocks
    //each segment of display has a direction, which is also the direction of blocks in it
    void block_sprawl(int i, int j, int sqrLim) {
        if (b[i][j].dse <= min_dense)return;
        int k;
        bool vis[32][32];
        memset(vis, 0, sizeof(vis));
        pl_vector p, u, v;
        p.set(i, j);
        vis[i][j] = true;
        std::queue<pl_vector> q;
        q.push(p);
        while (!q.empty()) {//BFS/Flood Fill
            u = q.front();
            q.pop();
            for (k = 0; k < 4; k++) {//how about space_x,y
                v.set(u.x + dirx[k], u.y + diry[k]);
                int sqrd = sqr_block_dis(p, v, co_x, co_y);
                if (block_exist(v.x, v.y) && b[v.x][v.y].dse && !vis[v.x][v.y] && sqrd <= sqrLim) {
                    q.push(v);
                    vis[v.x][v.y] = true;
                    pl_vector vec;
                    vec = b[v.x][v.y].dse *
                            (make_vec(b[v.x][v.y].cx, b[v.x][v.y].cy) - make_vec(b[i][j].cx, b[i][j].cy));
                    int dir = vec.basic_direction();
                    b[i][j].d[dir] = b[i][j].d[dir] + vec;
                }
            }
        }
    }

    void measureB() {
        int hv[32][32];//horizontal-vertical
        int display[7];
        memset(hv, 0, sizeof(hv));
        memset(display, 0, sizeof(display));
        int i, j;
        for (i = 0; i < bx; i++)
            for (j = 0; j < by; j++) {
                if (b[i][j].dse <= min_dense)continue;
                double modu[4];
                int mk = 0, k;//mk be the max modu's subscript
                for (k = 0; k < 4; k++) {
                    modu[k] = b[i][j].d[k].modulus();
                    if (modu[k] > modu[mk])mk = k;
                }
                //if one vecter(direction) is signifcantly larger than another
                if (modu[mk] > 1.6 * modu[(mk + 1) & 3] &&
                        modu[mk] > 1.6 * modu[(mk + 3) & 3]) {//measure normal direction, &3 = %4
                    if (mk & 1)hv[i][j] = 2;//vertical
                    else hv[i][j] = 1;//horizontal
                } else hv[i][j] = 3;//Not, view it as a joint
            }
        int sv[4] = {0, 0, 0, 0};//sum of vetical block in four parts
        //subscript 00,01,10,11 for BL,TL,BR,LR
        for (j = by - 1; j >= 0; j--)
            for (i = 0; i < bx; i++)
                if (hv[i][j] == 2 || hv[i][j] == 3) {
                    int addit;
                    if (hv[i][j] == 2)addit = 2; else addit = 1;
                    //++sv[((i<(bx>>1))<<1)|(j<(by>>1))];//just for fun
                    if (i < (bx >> 1)) {//depend on block_size
                        if (j < (by >> 1))sv[0] += addit;
                        else sv[1] += addit;
                    }
                    if (i > (bx >> 1)) {
                        if (j < (by >> 1))sv[2] += addit;
                        else sv[3] += addit;
                    }
                }
        if (sv[0] >= 4)display[4] = 1;//the "2" depend on block_size
        if (sv[1] >= 4)display[1] = 1;
        if (sv[2] >= 4)display[5] = 1;
        if (sv[3] >= 4)display[2] = 1;
        int sh[3] = {0, 0, 0};//sum of horizontal block in three parts
        //subscript 0,1,2 for T,M,B
        for (i = 0; i < bx; i++) {
            for (j = 0; j < by / 3; j++)if (hv[i][j] == 1)sh[0] += 2; else if (hv[i][j] == 3)sh[0] += 1;
            for (; j < (by << 1) / 3; j++)if (hv[i][j] == 1)++sh[1]; else if (hv[i][j] == 3)sh[1] += 1;
            for (; j < by; j++)if (hv[i][j] == 1)++sh[2]; else if (hv[i][j] == 3)sh[2] += 1;
        }
        if (sh[0] >= 7)display[6] = 1;
        if (sh[1] >= 7)display[3] = 1;
        if (sh[2] >= 7)display[0] = 1;
        if (!display[0] && !display[3] && !display[6])result = 1;
        else if (display[5])result = 3;
        else result = 2;
    }
};

#endif
