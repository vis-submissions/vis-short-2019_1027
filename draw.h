#pragma once

#include "image.h"

void draw_circle(
        int cx,
        int cy,
        int r,
        image_rgb8& img,
        RGB color = RGB{ 0, 0, 0 }
        );

void fill_circle(
        int cx,
        int cy,
        int r,
        image_rgb8& img,
        RGB color = RGB{ 0, 0, 0 }
        );

void draw_line(
        int x0,
        int y0,
        int x1,
        int x2,
        image_rgb8& img,
        RGB color = RGB{ 0, 0, 0 }
        );
