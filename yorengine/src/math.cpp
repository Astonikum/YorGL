#include "yorengine/math.hpp"

#include <cmath>

namespace yorengine {

float Vec3::lengthSquared() const noexcept {
    return dot(*this, *this);
}

float Vec3::length() const noexcept {
    return std::sqrt(lengthSquared());
}

Vec3 Vec3::normalized() const noexcept {
    const float squared = lengthSquared();
    if (squared <= 1.0e-12f) return {};
    return *this / std::sqrt(squared);
}

Quaternion Quaternion::fromEulerRadians(Vec3 radians) noexcept {
    const float halfX = radians.x * 0.5f;
    const float halfY = radians.y * 0.5f;
    const float halfZ = radians.z * 0.5f;
    const float sx = std::sin(halfX);
    const float cx = std::cos(halfX);
    const float sy = std::sin(halfY);
    const float cy = std::cos(halfY);
    const float sz = std::sin(halfZ);
    const float cz = std::cos(halfZ);

    return {
        sx * cy * cz - cx * sy * sz,
        cx * sy * cz + sx * cy * sz,
        cx * cy * sz - sx * sy * cz,
        cx * cy * cz + sx * sy * sz,
    };
}

Quaternion Quaternion::operator*(Quaternion other) const noexcept {
    return {
        w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y - x * other.z + y * other.w + z * other.x,
        w * other.z + x * other.y - y * other.x + z * other.w,
        w * other.w - x * other.x - y * other.y - z * other.z,
    };
}

Quaternion Quaternion::normalized() const noexcept {
    const float squared = x * x + y * y + z * z + w * w;
    if (squared <= 1.0e-12f) return identity();
    const float inverse = 1.0f / std::sqrt(squared);
    return {x * inverse, y * inverse, z * inverse, w * inverse};
}

Vec3 Quaternion::rotate(Vec3 value) const noexcept {
    const Quaternion q = normalized();
    const Quaternion vector{value.x, value.y, value.z, 0.0f};
    const Quaternion inverse{-q.x, -q.y, -q.z, q.w};
    const Quaternion result = q * vector * inverse;
    return {result.x, result.y, result.z};
}

Mat4::Mat4() noexcept : values_{} {
    values_[0] = 1.0f;
    values_[5] = 1.0f;
    values_[10] = 1.0f;
    values_[15] = 1.0f;
}

Mat4 Mat4::identity() noexcept {
    return {};
}

Mat4 Mat4::translation(Vec3 value) noexcept {
    Mat4 result;
    result.at(0, 3) = value.x;
    result.at(1, 3) = value.y;
    result.at(2, 3) = value.z;
    return result;
}

Mat4 Mat4::rotation(Quaternion value) noexcept {
    const Quaternion q = value.normalized();
    const float xx = q.x * q.x;
    const float yy = q.y * q.y;
    const float zz = q.z * q.z;
    const float xy = q.x * q.y;
    const float xz = q.x * q.z;
    const float yz = q.y * q.z;
    const float wx = q.w * q.x;
    const float wy = q.w * q.y;
    const float wz = q.w * q.z;

    Mat4 result;
    result.at(0, 0) = 1.0f - 2.0f * (yy + zz);
    result.at(0, 1) = 2.0f * (xy - wz);
    result.at(0, 2) = 2.0f * (xz + wy);
    result.at(1, 0) = 2.0f * (xy + wz);
    result.at(1, 1) = 1.0f - 2.0f * (xx + zz);
    result.at(1, 2) = 2.0f * (yz - wx);
    result.at(2, 0) = 2.0f * (xz - wy);
    result.at(2, 1) = 2.0f * (yz + wx);
    result.at(2, 2) = 1.0f - 2.0f * (xx + yy);
    return result;
}

Mat4 Mat4::scale(Vec3 value) noexcept {
    Mat4 result;
    result.at(0, 0) = value.x;
    result.at(1, 1) = value.y;
    result.at(2, 2) = value.z;
    return result;
}

float& Mat4::at(std::size_t row, std::size_t column) noexcept {
    return values_[column * 4 + row];
}

float Mat4::at(std::size_t row, std::size_t column) const noexcept {
    return values_[column * 4 + row];
}

Vec3 Mat4::transformPoint(Vec3 value) const noexcept {
    const float x = value.x * at(0, 0) + value.y * at(0, 1) + value.z * at(0, 2) + at(0, 3);
    const float y = value.x * at(1, 0) + value.y * at(1, 1) + value.z * at(1, 2) + at(1, 3);
    const float z = value.x * at(2, 0) + value.y * at(2, 1) + value.z * at(2, 2) + at(2, 3);
    return {x, y, z};
}

Mat4 Mat4::operator*(const Mat4& other) const noexcept {
    Mat4 result;
    result.values_.fill(0.0f);
    for (std::size_t row = 0; row < 4; ++row) {
        for (std::size_t column = 0; column < 4; ++column) {
            for (std::size_t k = 0; k < 4; ++k) {
                result.at(row, column) += at(row, k) * other.at(k, column);
            }
        }
    }
    return result;
}

Mat4 Transform::localMatrix() const noexcept {
    return Mat4::translation(position) * Mat4::rotation(rotation) * Mat4::scale(scale);
}

} // namespace yorengine
