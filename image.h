#pragma once

#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

template <typename T>
class image2d
{
public:
    using pixel_type = T;

public:
    image2d()
        : width_(0)
        , height_(0)
        , pixels_(0)
    {
    }

    image2d(int w, int h)
        : width_(w)
        , height_(h)
        , pixels_(w * h)
    {
    }

    image2d(image2d const& rhs)
        : width_(rhs.width_)
        , height_(rhs.height_)
        , pixels_(rhs.width_ * rhs.height_)
    {
        std::copy(rhs.pixels_.begin(), rhs.pixels_.end(), pixels_.begin());
    }

    int width() const
    {
        return width_;
    }

    int height() const
    {
        return height_;
    }

    pixel_type& operator()(int x, int y)
    {
        return pixels_[y * width_ + x];
    }

    pixel_type const& operator()(int x, int y) const
    {
        return pixels_[y * width_ + x];
    }

    pixel_type* data()
    {
        return pixels_.data();
    }

    pixel_type const* data() const
    {
        return pixels_.data();
    }

private:
    int width_;
    int height_;
    std::vector<pixel_type> pixels_;

};

struct RGB
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

using image_rgb8 = image2d<RGB>;

void save_image(std::string filename, image_rgb8 const& img);
