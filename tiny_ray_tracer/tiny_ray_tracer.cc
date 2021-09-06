#include <limits>
#include <fstream>
#include <vector>

#include "geometry.hpp"

struct Material {
    explicit constexpr Material() {}
    explicit constexpr Material(const geometry::vec4& a,const geometry::vec3& color, const float& spec, const float& r)
        : albedo(a),
          diffuse_color(color),
          specular_exponent(spec),
          refractive_index(r) {}

    geometry::vec4 albedo = { 1,0,0,0 };
    geometry::vec3 diffuse_color = {};
    float specular_exponent = {};
    float refractive_index = 1.0f;
};

struct Light {
    explicit constexpr Light(const geometry::vec3& p, const float& i) : position(p), intensity(i){}

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
    auto& intersection, auto& normal_vector, auto& material) {
    auto spheres_dist = std::numeric_limits<float>::max();
    for(auto& sphere : spheres) {
        float dist_i = 0;
        if (sphere.ray_intersect(orig, dir, dist_i) && dist_i < spheres_dist) {
            spheres_dist = dist_i;
            intersection = orig + dir * dist_i;
            normal_vector = (intersection - sphere.center).normalize();
            material = &sphere.material;
        }
    }

    float checker_board_dist = std::numeric_limits<float>::max();
    if (std::fabs(dir.y) > 1e-3) {
        float d = -(orig.y + 4) / dir.y;
        auto pt = orig + dir * d;
        if (d > 0 && std::fabs(pt.x) < 10 && pt.z<-10 && pt.z>-30 && d < spheres_dist) {
            checker_board_dist = d;
            intersection = pt;
            normal_vector = { 0,1.0f,0 };
            constinit static Material white = Material({ 1,0,0,0 }, geometry::vec3{ 1,1,1 } *0.3, 0, 1);
            constinit static Material yellow = Material({ 1,0,0,0 }, geometry::vec3{ 1,0.7,0.3 } *0.3, 0, 1);
            material = ((int(0.5 * intersection.x + 1000) + int(0.5 * intersection.z)) & 1) ?
                &white :
                &yellow;
            // I can't understand why the code below can't get a right image.
            //static Material m = Material();
            //m = Material();
            //if(((int(0.5 * intersection.x + 1000) + int(0.5 * intersection.z)) & 1)) {
            //    m.diffuse_color = geometry::vec3{ 1,1,1 } * 0.3;
            //}
            //else {
            //    m.diffuse_color = geometry::vec3{ 1,0.7,0.3 } * 0.3;
            //}
            //material = &m;
        }
    }
    return std::min(spheres_dist, checker_board_dist) < 1000;
};

auto cast_ray = [](const auto& orig, const auto& dir, 
                   const auto& spheres, const auto& lights,
                   size_t depth=0) {
    geometry::vec3 intersection, normal_vector;
    const Material* material;
    if (depth > 4 || !scene_intersect(orig,dir,spheres,intersection,normal_vector,material)) {
        return geometry::vec3{ 0.2f,0.7f,0.8f };
    }
    auto reflect = [](const auto& I, const auto& N) {
        return I - N * 2.0f * (I * N);
    };
    auto refract = [](const auto& I, const auto& N, const float& refractive_index) {
        float cosi = -std::max(-1.0f, std::min(1.0f, I * N));
        float etai = 1, etat = refractive_index;
        auto n = N;
        if(cosi < 0) {
            cosi = -cosi;
            std::swap(etai, etat);
            n = -N;
        }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 
            geometry::vec3{ 0,0,0 } :
            I * eta + n * (eta * cosi - std::sqrtf(k));
    };
    auto reflect_direction = reflect(dir, normal_vector).normalize();
    auto refract_direction = refract(dir, normal_vector, material->refractive_index).normalize();
    auto reflect_orig = reflect_direction * normal_vector < 0 ?
        intersection - normal_vector * 1e-3f:
        intersection + normal_vector * 1e-3f;
    auto refract_orig = refract_direction * normal_vector < 0 ?
        intersection - normal_vector * 1e-3f :
        intersection + normal_vector * 1e-3f;

    auto reflect_color = cast_ray(reflect_orig, reflect_direction, spheres, lights, depth + 1);
    auto refract_color = cast_ray(refract_orig, refract_direction, spheres, lights, depth + 1);
    float diffuse_light_intensity = 0, specular_light_intensity = 0;
    for(const auto& light : lights) {
        auto light_dir = (light.position - intersection).normalize();
        auto light_distance = (light.position - intersection).norm();
        auto shadow_orig = light_dir * normal_vector < 0 ? 
            intersection - normal_vector * 1e-3:
            intersection + normal_vector * 1e-3;
        geometry::vec3 shadow_intersection, shadow_nv;
        const Material* tmpmaterial;
        if(scene_intersect(shadow_orig, light_dir, spheres, shadow_intersection, shadow_nv, tmpmaterial) && 
           (shadow_intersection - shadow_orig).norm() < light_distance) {
            // shadow_vector = shadow_intersection - shadow_orig.
            // skip the light, the pixel would be dark, making the same effect as shadows.
            continue;
        }

        diffuse_light_intensity += light.intensity * std::max(0.0f, light_dir * normal_vector);
        specular_light_intensity += std::powf(std::max(0.0f, reflect(light_dir, normal_vector) * dir), material->specular_exponent)*light.intensity;
    }
    return material->diffuse_color * 
           diffuse_light_intensity * material->albedo[0] + 
           geometry::vec3{ 1.0f,1.0f,1.0f } * specular_light_intensity * material->albedo[1] + 
           reflect_color*material->albedo[2] + 
           refract_color*material->albedo[3];
};

auto render(const auto& spheres, const auto& lights) {
    constexpr int width = 1024;
    constexpr int height = 768;
    constexpr int fov = 3.1415926f / 2.;
    std::vector<geometry::vec3> frame_buffer(width * height);

    for(size_t j = 0; j<height; j++) {
        for(size_t i = 0; i<width; i++) {
            float x = (2 * (i + 0.5) / (float)width - 1) * tan(fov / 2.0) * width / (float)height;
            float y = -(2 * (j + 0.5) / (float)height - 1) * tan(fov / 2.0);
            auto dir = geometry::vec3{ x,y,-1 }.normalize();
            frame_buffer[i + j * width] = cast_ray(geometry::vec3{ 0,0,0 }, dir, spheres, lights);
        }
    }

    std::ofstream ofs;
    ofs.open("./out.ppm", std::ofstream::out | std::ofstream::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n";
    for(size_t i = 0; i < height * width; ++i) {
        for(size_t j = 0; j<3; j++) {
            auto& c = frame_buffer[i];
            float max = std::max(c[0], std::max(c[1], c[2]));
            if(max > 1) {
                c = c * (1.0 / max);
            }
            ofs << (char)(255 * std::max(0.0f, std::min(1.f, frame_buffer[i][j])));
        }
    }
    ofs.close();
}

auto main() -> int {
    constexpr Material ivory({0.6,0.3,0.1,0}, { 0.4f, 0.4f, 0.3f }, 50.0f ,1.0f);
    constexpr Material red_rubber({0.9,0.1,0, 0}, { 0.3f, 0.1f, 0.1f },10.0f, 1.0f);
    constexpr Material mirror({0,10.0,0.8, 0}, { 1.0f,1.0f,1.0f },1425.0f,1.0f);
    constexpr Material glass({ 0,0.5,0.1,0.8 }, { 0.6f,0.7f,0.8f }, 125.0f, 1.5f);

    constexpr Sphere sphere1(geometry::vec3{ -3, 0, -16 }, 2, ivory);
    constexpr Sphere sphere2(geometry::vec3{ -1, -1.5, -12 }, 2, glass);
    constexpr Sphere sphere3(geometry::vec3{ 1.5, -0.5, -18 }, 3, red_rubber);
    constexpr Sphere sphere4(geometry::vec3{ 7, 5, -18 }, 4, mirror);

    // visual studio bug
    // https://developercommunity2.visualstudio.com/t/Cant-declare-constexpr-initializer_list/668718
    //constexpr std::vector spheres({ sphere1, sphere2, sphere3, sphere4 });
    std::vector spheres({ sphere1, sphere2, sphere3, sphere4 });
    std::vector lights = {
        Light({-20,20,20},1.5),
        Light({30.0,50.0,-25},1.8),
        Light({30.0,20.0,30.0},1.7)
    };
    render(spheres, lights);

    // constexpr test field
    {
        constexpr auto x = gm::vec3{ 1,2,3 };
        static_assert(x[0] == 1.0f, "hello wrong");
    }
    return 0;
}