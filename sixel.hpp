#ifndef TINYSIXEL_SIXEL_HPP
#define TINYSIXEL_SIXEL_HPP

#include <climits>

#include <algorithm>
#include <ostream>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

class SixelImage {
private:
    std::vector<std::string> escaped_;

private:
    static void print_times(std::ostream &os, int val, int cnt)
    {
        char ch = static_cast<char>(val);
        switch (cnt) {
        case 3:
            os << ch;
        case 2:
            os << ch;
        case 1:
            os << ch;
            break;

        default:
            os << "!" << cnt;
            os << ch;
            break;
        }
    }

    static std::vector<std::string> escape(int width, int height,
                                           const uint8_t *pixels)
    {
        // Thanks to:
        // ftp://ftp.fu-berlin.de/unix/www/lynx/pub/shuford/terminal/all_about_sixels.txt

        // ? ... ~
        // - : LF (beginning of the next line)
        // $ : CR (beginning of the current line)
        // #0;2;r;g;b : color

        std::vector<std::string> escaped_lines;

        for (int ny = 0; ny < (height + 5) / 6; ny++) {
            int y = ny * 6;

            std::stringstream ss;
            std::vector<std::tuple<int, int, int, int, int>> colorpos;

            // Construct color2pos map.
            for (int x = 0; x < width; x++) {
                for (int i = 0; i < 6; i++) {
                    int pos = ((y + i) * width + x) * 4;
                    if (y + i >= height) break;
                    int red = pixels[pos + 0], green = pixels[pos + 1],
                        blue = pixels[pos + 2], alpha = pixels[pos + 3];

                    if (alpha == 0) continue;
                    red *= (alpha / 255.f) * 100 / 255;
                    green *= (alpha / 255.f) * 100 / 255;
                    blue *= (alpha / 255.f) * 100 / 255;

                    colorpos.push_back({red, green, blue, x, i});
                }
            }

            std::sort(colorpos.begin(), colorpos.end());

            // Do actual printing.
            auto cpit = colorpos.begin();
            while (cpit != colorpos.end()) {
                int red = std::get<0>(*cpit), green = std::get<1>(*cpit),
                    blue = std::get<2>(*cpit);
                auto end_cpit =
                    std::upper_bound(colorpos.begin(), colorpos.end(),
                                     std::tuple<int, int, int, int, int>(
                                         red, green, blue, INT_MAX, INT_MAX));
                // [cpit, end_cpit) has the same color.

                // Go to the head of the line.
                ss << "$";
                // Set color.
                ss << "#10;2;" << red << ";" << green << ";" << blue;

                int val0 = 0, cnt = 0;
                for (int x = 0; x < width; x++) {
                    int val = 0;
                    for (; cpit != end_cpit && std::get<3>(*cpit) == x; ++cpit)
                        val |= (1 << std::get<4>(*cpit));
                    val += '?';

                    if (cnt == 0 || val == val0) {
                        val0 = val;
                        ++cnt;
                        continue;
                    }

                    print_times(ss, val0, cnt);

                    cnt = 1;
                    val0 = val;
                }
                print_times(ss, val0, cnt);
            }

            // Go to the next line.
            ss << "-";

            escaped_lines.push_back(ss.str());
        }

        return escaped_lines;
    }

public:
    SixelImage(int width, int height, const uint8_t *pixels)
        : escaped_(escape(width, height, pixels))
    {
    }

    const std::vector<std::string> &getEscaped() const
    {
        return escaped_;
    }
};

class Sixel {
private:
    std::ostream &os_;

public:
    Sixel(std::ostream &os) : os_(os)
    {
    }

    void print(const SixelImage &image)
    {
        os_ << enter();

        const std::vector<std::string> &src = image.getEscaped();
        for (auto &&s : src) os_ << s;

        os_ << exit() << std::flush;
    }

private:
    static const char *enter()
    {
        return "\033P0;0;8q\"1;1";
    }

    static const char *exit()
    {
        return "\033\\";
    }
};

#endif
