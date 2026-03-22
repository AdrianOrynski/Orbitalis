#include "Vector3.h"
#include <cmath>

Vector3::Vector3() : x(0), y(0), z(0) {}

Vector3::Vector3(double x, double y, double z)
    : x(x), y(y), z(z) {
}

Vector3 Vector3::operator+(const Vector3& other) const
{
    return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::operator-(const Vector3& other) const
{
    return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 Vector3::operator*(double scalar) const
{
    return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3 Vector3::operator/(double scalar) const
{
    return Vector3(x / scalar, y / scalar, z / scalar);
}

Vector3& Vector3::operator+=(const Vector3& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vector3& Vector3::operator*=(double scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

double Vector3::Length() const
{
    return std::sqrt(x * x + y * y + z * z);
}

Vector3 Vector3::Normalized() const
{
    double len = Length();
    if (len == 0.0)
        return Vector3(0, 0, 0);

    return Vector3(x / len, y / len, z / len);
}

Vector3 operator*(double scalar, const Vector3& v)
{
    return Vector3(v.x * scalar, v.y * scalar, v.z * scalar);
}