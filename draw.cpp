#include <cmath>
#include <iostream>
#include <utility>

#include "draw.h"

inline int clamp(int x, int a, int b)
{
    return std::min(std::max(x, a), b);
}

void draw_circle(
        int dx,
        int dy,
        int r,
        image_rgb8& img,
        RGB color
        )
{
    for (int i = 0; i < 360; i += 4)
    {
        float rad0 = i / 180.0f * M_PI;
        float rad1 = (i + 4) / 180.0f * M_PI;

        float x0 = dx + std::cos(rad0) * r;
        float y0 = dy - std::sin(rad0) * r;
        float x1 = dx + std::cos(rad1) * r;
        float y1 = dy - std::sin(rad1) * r;

        draw_line(x0, y0, x1, y1, img, color);
    }
}

void fill_circle(
        int dx,
        int dy,
        int r,
        image_rgb8& img,
        RGB color
        )
{
    for (int y = std::max(0, dy - r); y <= std::min(img.height() - 1, dy + r); ++y)
    {
        for (int x = std::max(0, dx - r); x <= std::min(img.width() - 1, dx + r); ++x)
        {
            float dx2 = (x - dx) * (x - dx);
            float dy2 = (y - dy) * (y - dy);
            float dist = sqrtf(dx2 + dy2);

            if (dist <= r)
            {
                img(x, y) = color;
            }
        }
    }
}

void draw_line(
        int x0,
        int y0,
        int x1,
        int y1,
        image_rgb8& img,
        RGB color
        )
{
    x0 = clamp(x0, 0, img.width());
    x1 = clamp(x1, 0, img.width());
    y0 = clamp(y0, 0, img.height());
    y1 = clamp(y1, 0, img.height());

    if (x0 == x1)
    {
        for (int y = std::min(y0, y1); y <= std::max(y0, y1); ++y)
        {
            img(x0, y) = color;
        }

        return;
    }

    if (y0 == y1)
    {
        for (int x = std::min(x0, x1); x <= std::max(x0, x1); ++x)
        {
            img(x, y0) = color;
        }

        return;
    }

    int dx = x1 - x0;
    int dy = y1 - y0;

    bool right = dx > 0;
    bool down = dy > 0;

    if (!right) dx = -dx;
    if (down) dy = -dy;

    int err = dx + dy;

    int x;
    int y;

    for (x = x0, y = y0; x != x1 || y != y1; )
    {
        img(x, y) = color;

        int tmp = err * 2;

        if (tmp > dy)
        {
            err += dy;
            x += right ? 1 : -1;
        }

        if (tmp < dx)
        {
            err += dx;
            y += down ? 1 : -1;
        }
    }
}
