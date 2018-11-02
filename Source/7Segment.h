#pragma once

#include <algorithm>
#include <cstring>
#include <deque>
#include <vector>
#include "Math/Vector.h"
#include <opencv2/core/mat.hpp>

#define sqr(a) ((a)*(a))

namespace {
    constexpr Vec2i directions[4] {{1, 0}, {0, 1}, {-1, 0}, {0, -1}}; //4 directions up,down,left,right
    const int block_count_W_log2 = 3;//sizeW,L=width,height>>block_size
    const int block_count_H_log2 = 4;//
    constexpr Vec2i co {1, 2};//block_count_H_log2 is 2*block_count_W_log2
    const int block_sqrLim = 36;//shoule be adjusted along with block_size & co.x,y
    const int min_dense = 2;//the min density for a block to sprawl
    const int verti_limit = 6;
    const int horiz_limit = 8;
    //consistent with dirx,y
    int basic_direction(Vec2i v) {
        //consider line y=x y=-x and the four parts they cut out
        if (v.x>=v.y && v.x>-v.y)return 0;
        if (v.x>v.y && v.x<=-v.y)return 3;
        if (v.x<=v.y && v.x<-v.y)return 2;
        return 1;
    }
    
    struct Block {
        Vec2i loc {0, 0};//location bottomleft
        Vec2i size {0, 0};
        Vec2i centre {0, 0};//mass centre
        Vec2i d[4] {{0, 0}, {0, 0}, {0, 0}, {0, 0}};
        int density = 0;//density
    };
    
    class Img_segment7 {
    public:
        void init() {
            height = width = 0;
            result = -1;
            bx = by = 0;
        }
        //notice that the form of input image is:x-axis right,y-axis down
        void input_image(cv::Mat1b matrix) {
            height = matrix.rows;
            width = matrix.cols;
            img = std::move(matrix);
        }
        
        void cv_identify() {
            //L,R,T,B for the range of the process in the image
            int L = width-1;
            int R = 0;
            int T = 0;
            int B = height-1;
            //get range
            for (int i = 0; i<width; i++)
                for (int j = 0; j<height; j++)
                    if (img(j, i)) {
                        if (i<L)L = i;
                        if (i>R)R = i;
                        if (j<B)B = j;
                        if (j>T)T = j;
                    }
            if (L>R || B>T) {
                result = -1;
                return;
            }
            //Wth,Lth for the width & height of the process area
            auto Wth = R-L;
            auto Lth = T-B;
            //LBx,LBy location bottonleft
            //A Stick
            if (Lth>=4*Wth) {
                result = 1;
                return;
            }
            //dx,size.y for the size of each block
            //divide the image into small blocks for process
            //Notice that in cv03 the dx=2*size.y because the display is such size
            auto block_size = Vec2i{Wth >> block_count_W_log2, Lth >> block_count_H_log2};
            //bx,by for the size of b[][], blocks
            bx = Wth/block_size.x+1;
            by = Lth/block_size.y+1;
            //process b[][], blocks
            for (int i = 0; i<bx; i++)
                for (int j = 0; j<by; j++) {
                    b[i][j].loc = Vec2i(i*block_size.x+L, j*block_size.y+B);
                    b[i][j].size = block_size;
                    block_process(&b[i][j]);
                }
            for (int i = 0; i<bx; i++)
                for (int j = 0; j<by; j++) {
                    block_sprawl(i, j, block_sqrLim);
                }
            measureB();
        }
        int get_result() {
            return result;
        }
        cv::Mat1b img;
    private:
        int height, width;
        int result;
        Block b[32][32];
        int bx, by;

        //end illustration
        bool in_img(int x, int y) {
            return x>=0 && x<width && y>=0 && y<height;
        }

        //measure the actual distance that function "sprawl" travels when there is coefficient co.x,y
        int sqr_block_dis(Vec2i a, Vec2i b, int cox, int coy) {
            return sqr(coy)*sqr(a.x-b.x)+sqr(cox)*sqr(a.y-b.y);
        }
        //get basic info of the block
        void block_process(Block* b) {
            Vec2i v {0, 0}, loc = b->loc;
            const auto bound = b->loc + b->size;
            for (; loc.x < bound.x; ++loc.x)
                for (; loc.y < bound.y; ++loc.y)
                    if (in_img(loc.x, loc.y) && img(loc.y, loc.x)) {
                        ++b->density;
                        v += loc - b->loc;
                    }
            if (b->density)
                b->centre = b->loc+v/b->density;
        }
        //b[i][j] is in process area
        bool block_exist(int i, int j) {
            return i>=0 && i<bx && j>=0 && j<by;
        }
        //get the vectors, say, direction of a block considering its adjacent blocks
        //each segment of display has a direction, which is also the direction of blocks in it
        void block_sprawl(int i, int j, int sqrLim) {
            if (b[i][j].density<=min_dense)return;
            bool vis[32][32];
            memset(vis, 0, sizeof(vis));
            Vec2i p = {i, j};
            vis[i][j] = true;
            std::deque<Vec2i> q {p};
            while (!q.empty()) {//BFS/Flood Fill
                const auto u = q.front();
                q.pop_front();
                for (const auto x : directions) {//how about space.x,y
                    const auto v = u + x;
                    int sqrd = sqr_block_dis(p, v, co.x, co.y);
                    if (block_exist(v.x, v.y) && b[v.x][v.y].density && !vis[v.x][v.y] && sqrd<=sqrLim) {
                        q.push_back(v);
                        vis[v.x][v.y] = true;
                        const auto vec = (b[v.x][v.y].centre - b[i][j].centre) * b[v.x][v.y].density;
                        b[i][j].d[basic_direction(vec)] += vec;
                    }
                }
            }
        }
        void measureB() {
            int hv[32][32];//horizontal-vertical
            bool display[7];
            memset(hv, 0, sizeof(hv));
            memset(display, 0, sizeof(display));
            int i, j;
            for (i = 0; i<bx; i++)
                for (j = 0; j<by; j++) {
                    if (b[i][j].density<=min_dense)continue;
                    double modu[4];
                    int mk = 0, k;//mk be the max modu's subscript
                    for (k = 0; k<4; k++) {
                        modu[k] = b[i][j].d[k].length();
                        if (modu[k]>modu[mk])mk = k;
                    }
                    //if one vecter(direction) is signifcantly larger than another
                    if (modu[mk]>1.6*modu[(mk+1) & 3]
                            && modu[mk]>1.6*modu[(mk+3) & 3]) {//measure normal direction, &3 = %4
                        if (mk & 1)hv[i][j] = 2;//vertical
                        else hv[i][j] = 1;//horizontal
                    }
                    else hv[i][j] = 3;//Not, view it as a joint
                }
            int sv[4] = {0, 0, 0, 0};//sum of vetical block in four parts
            //subscript 00,01,10,11 for BL,TL,BR,LR
            for (j = by-1; j>=0; j--)
                for (i = 0; i<bx; i++)
                    if (hv[i][j]==2 || hv[i][j]==3) {
                        int addit;
                        if (hv[i][j]==2)addit = 2; else addit = 1;
                        if (i<(bx >> 1)) {//depend on block_size
                            if (j<(by >> 1))sv[0] += addit;
                            else sv[1] += addit;
                        }
                        if (i>(bx >> 1)) {
                            if (j<(by >> 1))sv[2] += addit;
                            else sv[3] += addit;
                        }
                    }
            if (sv[0]>=verti_limit)display[4] = true;//the "2" depend on block_size
            if (sv[1]>=verti_limit)display[1] = true;
            if (sv[2]>=verti_limit)display[5] = true;
            if (sv[3]>=verti_limit)display[2] = true;
            int sh[3] = {0, 0, 0};//sum of horizontal block in three parts
            //subscript 0,1,2 for T,M,B
            for (i = 0; i<bx; i++) {
                for (j = 0; j<by/3; j++)if (hv[i][j]==1)sh[0] += 2; else if (hv[i][j]==3)sh[0] += 1;
                for (; j<(by << 1)/3; j++)if (hv[i][j]==1)sh[1] += 2; else if (hv[i][j]==3)sh[1] += 1;
                for (; j<by; j++)if (hv[i][j]==1)sh[2] += 2; else if (hv[i][j]==3)sh[2] += 1;
            }
            if (sh[0]>=horiz_limit)display[6] = true;
            if (sh[1]>=horiz_limit)display[3] = true;
            if (sh[2]>=horiz_limit)display[0] = true;
            std::printf("%d:%d:%d:%d:%d:%d:%d", display[0], display[1], display[2], display[3],display[4], display[5], display[6]);
            if (!display[1] && !display[2] && !display[4] && !display[5])result = -1;
            else if (!display[0] && !display[3] && !display[6])result = 1;
            else if (display[4] && display[5]) {
                if (sv[0]>=sv[2])result = 2;
                else result = 3;
            }
            else if (display[5])result = 3;
            else result = 2;
        }
    };
}
