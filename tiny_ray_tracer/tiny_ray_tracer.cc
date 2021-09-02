#include <limits>
#include <fstream>
#include <vector>

#include "geometry.hpp"

struct Sphere {
    Sphere(const geometry::vec3& c, const float& r): center(c), radius(r) {
        
    }

    auto ray_intersect(const geometry::vec3& orig, const geometry::vec3& dir, float& t0) const {
        geometry::vec3 L = center - orig;
        float tca = L * dir;
        float d2 = L * L - tca * tca;
        if(d2 > radius*radius) {
            return false;
        }
        float thc = sqrtf(radius * radius - d2);
        t0 = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
    geometry::vec3 center;
    float radius;
};

auto cast_ray = [](const auto& orig, const auto& dir, const auto& sphere) {
    auto sphere_dist = std::numeric_limits<float>::max();
    if(!sphere.ray_intersect(orig,dir,sphere_dist)) {
        return geometry::vec3{ 0.2,0.7,0.8 };
    }
    return geometry::vec3{ 0.4, 0.4, 0.3 };
};

auto render(const auto& sphere) {
    constexpr int width = 1024;
    constexpr int height = 768;
    constexpr float fov = 3.1415926 / 3.0f;
    std::vector<geometry::vec3> frame_buffer(width * height);

    for(size_t j = 0; j<height; j++) {
        for(size_t i = 0; i<width; i++) {
            float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.0) * width / (float)height;
            float y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.0);
            auto dir = geometry::vec3{ x,y,-1 }.normalize();
            frame_buffer[i + j * width] = cast_ray(geometry::vec3{ 0,0,0 }, dir, sphere);
        }
    }

    std::ofstream ofs;
    ofs.open("./out.ppm");
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for(size_t i = 0; i < height * width; ++i) {
        for(size_t j = 0; j<3; j++) {
            ofs << (char)(255 * std::max(0.f, std::min(1.f, frame_buffer[i][j])));
        }
    }
    ofs.close();
}

auto main() -> int {
    Sphere sphere(geometry::vec3{ 0, 0, -16 }, 2);
    render(sphere);
    constexpr auto x = gm::vec3{ 1,2,3 };
    static_assert(x[0] == 1.0f, "");
    return 0;
}