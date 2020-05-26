#include "image.h"

void save_image(std::string filename, image_rgb8 const& img)
{
    std::ofstream file(filename);

    int pitch = img.width() * 3;

    file << "P6\n";
    file << img.width() << ' ' << img.height() << '\n';
    file << 255 << '\n';

    file.write(reinterpret_cast<char const*>(img.data()), pitch * img.height());
}
