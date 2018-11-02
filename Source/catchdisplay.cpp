#include <iostream>
#include <deque>
#include <utility>
#include "Math/Vector.h"
#include "FrameProcessor.h"
#include "7Segment.h"
#include <opencv2/imgproc.hpp>
using namespace std;
namespace {
    const int sdirx[12] = {1, 0, -1, 0, 2, 1, 0, -1, -2, -1, 0, 1};
    const int sdiry[12] = {0, 1, 0, -1, 0, -1, -2, -1, 0, 1, 2, 1};
    const int thres_min_R = 190;
    const int thres_max_G = 160;
    const int thres_max_B = 160;
    const int dis_max_R = 100;
    const int dis_max_G = 100;
    const int dis_max_B = 100;
    const int const_catch = 256;
    const int const_sample = 128;
    bool vis[const_catch][const_catch];

    int dsize, w, h;

    struct Rect {
        Vec2i min;
        Vec2i max;
    };

    void clip_rect(Rect& op, const Rect& bound) {
        if (op.min.x<bound.min.x) op.min.x = bound.min.x;
        if (op.min.y<bound.min.y) op.min.y = bound.min.y;
        if (op.max.x>bound.max.x) op.max.x = bound.max.x;
        if (op.max.y>bound.max.y) op.max.y = bound.max.y;
    }

    auto R(cv::Vec3b i) noexcept { return i[2]; }
    auto G(cv::Vec3b i) noexcept { return i[1]; }
    auto B(cv::Vec3b i) noexcept { return i[0]; }

    class selector {
    public:
        explicit selector(const cv::Mat& input)
                :_input(input), _masked(input.rows, input.cols), _scaled() {
            auto view = input.begin<cv::Vec3b>();
            for (auto&& x : _masked) {
                auto pixel = *(view++);
                x = static_cast<uchar>((R(pixel)>thres_min_R && G(pixel)<thres_max_G && B(pixel)<thres_max_B) * 255);
            }
            dsize = std::max(input.rows, input.cols)/const_catch+1;
            auto scale = 1.0 / dsize;
            cv::resize(_masked, _scaled, cv::Size(), scale, scale, cv::InterpolationFlags::INTER_NEAREST);
            h = _scaled.rows;
            w = _scaled.cols;
        }

        std::pair<Rect, bool> extract_digit_rect(const Vec2i& base_point) {
            static constexpr int threshold = 20;
            int possible_point_count = 0;
            Rect ret{base_point, base_point};
            std::deque<Vec2i> q{base_point};
            while (!q.empty()) {
                auto u = q.front();
                q.pop_front();
                ++possible_point_count;
                for (int k = 0; k<12; k++) {
                    int vx = u.x+sdirx[k];
                    int vy = u.y+sdiry[k];
                    if (in_sample(vx, vy) && !vis[vx][vy] && _scaled(vy, vx)) {
                        vis[vx][vy] = true;
                        ret.min = {std::min(ret.min.x, vx), std::min(ret.min.y, vy)};
                        ret.max = {std::max(ret.max.x, vx), std::max(ret.max.y, vy)};
                        q.emplace_back(vx, vy);
                    }
                }
                if (q.empty()) {
                    for (int y = ret.min.y; y<ret.max.y; y++)
                        for (int x = ret.min.x; x<ret.max.x; x++)
                            if (!vis[x][y]) {
                                vis[x][y] = true;
                                if (_scaled(y, x))
                                    q.emplace_back(x, y);
                            }
                }
            }
            return {ret, possible_point_count>threshold};
        }

        bool evaluate_background_is_dark(const Rect& rect) {
            cv::Mat3b reduce_row, reduce_pix;
            auto area = cv::Mat3b(_input, cv::Range(rect.min.y, rect.max.y), cv::Range(rect.min.x, rect.max.x));
            cv::reduce(area, reduce_row, 0, cv::REDUCE_AVG);
            cv::reduce(reduce_row, reduce_pix, 1, cv::REDUCE_AVG);
            auto& pix = *reduce_pix.begin();
            return R(pix)<dis_max_R && G(pix)<dis_max_G && B(pix)<dis_max_B;
        }

        void compress_sample(Img_segment7* s, const Rect& area) {
            auto size = area.max-area.min;
            auto step = 1.0/(std::max(size.x, size.y)/const_sample+1);
            cv::Mat1b compressed;
            cv::resize(cv::Mat1b(_masked, cv::Range(area.min.y, area.max.y), cv::Range(area.min.x, area.max.x)),
                    compressed, cv::Size(), step, step);
            s->input_image(compressed);
        }

        bool get_sample(const Vec2i& base_point, Img_segment7* s) {
            vis[base_point.x][base_point.y] = true;
            // try to get the largest sample area
            Rect area {{0,0}, {0,0}};
            auto&&[rect, success] = extract_digit_rect(base_point);
            if (!success) return false;
            //if (!evaluate_background_is_dark(rect)) return false;
            auto sample_area = Rect{(rect.min-Vec2i{1, 1})*dsize, (rect.max+Vec2i{1, 1})*dsize};
            clip_rect(sample_area, Rect{Vec2i(0, 0), Vec2i(_input.cols, _input.rows)});
            compress_sample(s, sample_area);
            return true;
        }

        bool test_sample_centre(int x, int y) noexcept { return !vis[x][y] && _scaled(y, x); }

        bool in_sample(int x, int y) noexcept { return x>=0 && x<_scaled.cols && y>=0 && y<_scaled.rows; }
        const cv::Mat& _input;
        cv::Mat1b _masked, _scaled;
    };

}

cv::Mat ProcessFrame(const cv::Mat& input) {
    memset(vis, 0, sizeof(vis));
    Img_segment7 s{};
    selector select {input};
    s.init();
    for (int j = 0; j<h; j++)
        for (int i = 0; i<w; i++)
            if (select.test_sample_centre(i, j)) {
                if (select.get_sample({i, j}, &s)) {
                    s.cv_identify();
                    std::cout << s.get_result() << std::endl;
                    return s.img;
                }
            }
    return select._scaled;// cv::Mat1b(1, 1);
}
