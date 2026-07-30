#pragma once

#include <array>
#include <cstddef>

namespace yorengine {

struct Vec3 {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    constexpr Vec3 operator+(Vec3 other) const noexcept { return {x + other.x, y + other.y, z + other.z}; }
    constexpr Vec3 operator-(Vec3 other) const noexcept { return {x - other.x, y - other.y, z - other.z}; }
    constexpr Vec3 operator*(float scalar) const noexcept { return {x * scalar, y * scalar, z * scalar}; }
    constexpr Vec3 operator/(float scalar) const noexcept { return {x / scalar, y / scalar, z / scalar}; }
    constexpr Vec3& operator+=(Vec3 other) noexcept {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    float lengthSquared() const noexcept;
    float length() const noexcept;
    Vec3 normalized() const noexcept;

    static constexpr float dot(Vec3 left, Vec3 right) noexcept {
        return left.x * right.x + left.y * right.y + left.z * right.z;
    }

    static constexpr Vec3 cross(Vec3 left, Vec3 right) noexcept {
        return {
            left.y * right.z - left.z * right.y,
            left.z * right.x - left.x * right.z,
            left.x * right.y - left.y * right.x,
        };
    }
};

struct Quaternion {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    float w = 1.0f;

    static constexpr Quaternion identity() noexcept { return {}; }
    static Quaternion fromEulerRadians(Vec3 radians) noexcept;

    Quaternion operator*(Quaternion other) const noexcept;
    Quaternion normalized() const noexcept;
    Vec3 rotate(Vec3 value) const noexcept;
};

class Mat4 {
public:
    Mat4() noexcept;

    static Mat4 identity() noexcept;
    static Mat4 translation(Vec3 value) noexcept;
    static Mat4 rotation(Quaternion value) noexcept;
    static Mat4 scale(Vec3 value) noexcept;

    float& at(std::size_t row, std::size_t column) noexcept;
    float at(std::size_t row, std::size_t column) const noexcept;
    Vec3 transformPoint(Vec3 value) const noexcept;

    Mat4 operator*(const Mat4& other) const noexcept;

private:
    std::array<float, 16> values_{};
};

struct Transform {
    Vec3 position{};
    Quaternion rotation = Quaternion::identity();
    Vec3 scale{1.0f, 1.0f, 1.0f};

    Mat4 localMatrix() const noexcept;
};

} // namespace yorengine
