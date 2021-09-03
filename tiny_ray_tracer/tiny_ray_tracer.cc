#include <limits>
#include <fstream>
#include <random>
#include <vector>

#include "geometry.hpp"

struct Material {
    explicit constexpr Material() : diffuse_color() {}
    explicit constexpr Material(const geometry::vec3& color) : diffuse_color(color) {}
    geometry::vec3 diffuse_color;
};

struct Light {
    explicit constexpr Light(const geometry::vec3& p, const float& i) : position(p), intensity(i) {}

    geometry::vec3 position;
    float intensity;
};

struct Sphere {
    explicit constexpr Sphere(const geometry::vec3& c, const float& r, const Material& m)
        : center(c), radius(r), material(m) {

    }

    auto ray_intersect(const geometry::vec3& orig, const geometry::vec3& dir, float& t0) const {
        geometry::vec3 L = center - orig;
        float tca = L * dir;
        float d2 = L * L - tca * tca;
        if (d2 > radius * radius) {
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
        for (auto& sphere : spheres) {
            float dist_i = 0.0f;
            if (sphere.ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
                spheres_dist = dist_i;
                hit = orig + dir * dist_i;
                N = (hit - sphere.center).normalize();
                material = &sphere.material;
            }
        }
        return spheres_dist < 1000;
};

auto rand_f()
{
    static std::random_device device;
    static std::mt19937 generator(device());
    static std::uniform_real_distribution<float> random(-0.5, 0.5);
    return random(generator);
}

auto cast_ray = [](const auto& orig, const auto& dir, const auto& spheres, const auto& lights) {
    geometry::vec3 hit_point, N;
    const Material* material;
    if (!scene_intersect(orig, dir, spheres, hit_point, N, material)) {
        return geometry::vec3{ 0.2f,0.7f,0.8f };
    }
    float diffuse_light_intensity = 0;
    for (const auto& light : lights) {
        auto light_dir = (light.position - hit_point).normalize();
        diffuse_light_intensity += light.intensity * std::max(0.0f, light_dir * N);
    }
    return (*material).diffuse_color * diffuse_light_intensity;
};

auto render(const auto& spheres, const auto& lights) {
    constexpr int width = 1024;
    constexpr int height = 768;
    constexpr float fov = 3.1415926f / 2.;
    std::vector<geometry::vec3> frame_buffer(width * height);

    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            constinit static auto spp = 10;
            for(size_t k = 0; k < spp; k++) {
                auto rand_value = rand_f();
                float x = (2 * (i + 0.5 + rand_value)  / width  - 1) * tan(fov / 2.0) * width / (float)height;
                float y = -(2 * (j + 0.5 + rand_value) / height - 1) * tan(fov / 2.0);
                auto dir = geometry::vec3{ x,y,-1 }.normalize();
                frame_buffer[i + j * width] += cast_ray(geometry::vec3{ 0,0,0 }, dir, spheres, lights);
            }
            frame_buffer[i + j * width] /= spp;
        }
    }

    std::ofstream ofs;
    ofs.open("./out.ppm", std::ofstream::out | std::ofstream::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for (size_t i = 0; i < height * width; ++i) {
        for (size_t j = 0; j < 3; j++) {
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
    std::vector lights = { Light({-20,20,20},1.5) };
    render(spheres, lights);

    // constexpr test field
    {
        constexpr auto x = gm::vec3{ 1,2,3 };
        static_assert(x[0] == 1.0f, "hello wrong");
    }
    return 0;
}