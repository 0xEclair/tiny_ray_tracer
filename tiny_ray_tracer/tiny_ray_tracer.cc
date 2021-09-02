#include <limits>
#include <fstream>
#include <vector>

#include "geometry.hpp"

struct Material {
    explicit constexpr Material() : diffuse_color() {}
    explicit constexpr Material(const geometry::vec3& color) :diffuse_color(color) {}
    geometry::vec3 diffuse_color;
};

struct Sphere {
    explicit constexpr Sphere(const geometry::vec3& c, const float& r, const Material& m)
        : center(c), radius(r), material(m) {
        
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
    Material material;
};

auto scene_intersect = [](
    const auto& orig, const auto& dir, const auto& spheres, 
    auto& hit, auto& N, auto& material) {
        auto spheres_dist = std::numeric_limits<float>::max();
        for(size_t i = 0; i<spheres.size(); i++) {
            if(float dist_i; spheres[i].ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
                spheres_dist = dist_i;
                hit = orig + dir * dist_i;
                N = (hit - spheres[i].center).normalize();
                material = spheres[i].material;
            }
        }
        return spheres_dist < 1000;
};

auto cast_ray = [](const auto& orig, const auto& dir, const auto& spheres) {
    geometry::vec3 point, N;
    Material material;
    if (!scene_intersect(orig,dir,spheres,point,N,material)) {
        return geometry::vec3{ 0.2,0.7,0.8 };
    }
    return geometry::vec3{ 0.4, 0.4, 0.3 };
};

auto render(const auto& spheres) {
    constexpr int width = 1024;
    constexpr int height = 768;
    constexpr float fov = 3.1415926 / 3.0f;
    std::vector<geometry::vec3> frame_buffer(width * height);

    for(size_t j = 0; j<height; j++) {
        for(size_t i = 0; i<width; i++) {
            float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.0) * width / (float)height;
            float y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.0);
            auto dir = geometry::vec3{ x,y,-1 }.normalize();
            frame_buffer[i + j * width] = cast_ray(geometry::vec3{ 0,0,0 }, dir, spheres);
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
    constexpr Material ivory({ 0.4f,0.4f,0.3f });
    constexpr Material red_rubber({ 0.3f,0.1f,0.1f });

    constexpr Sphere sphere1(geometry::vec3{ -3, 0, -16 }, 2, ivory);
    constexpr Sphere sphere2(geometry::vec3{ -1.0, -1.5, -12 }, 2, red_rubber);
    constexpr Sphere sphere3(geometry::vec3{ 1.5, -0.5, -18 }, 3, red_rubber);
    constexpr Sphere sphere4(geometry::vec3{ 7, 5, -18 }, 4, ivory);

    // visual studio bug
    // https://developercommunity2.visualstudio.com/t/Cant-declare-constexpr-initializer_list/668718
    //constexpr std::vector spheres({ sphere1, sphere2, sphere3, sphere4 });
    std::vector spheres({ sphere1, sphere2, sphere3, sphere4 });
    render(spheres);

    // constexpr test field
    {
        constexpr auto x = gm::vec3{ 1,2,3 };
        static_assert(x[0] == 1.0f, "hello wrong");
    }
    return 0;
}