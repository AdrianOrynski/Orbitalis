#pragma once

struct Vector3
{
    double x;
    double y;
    double z;

    Vector3();
    Vector3(double x, double y, double z);

    // Operators
    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-(const Vector3& other) const;
    Vector3 operator*(double scalar) const;
    Vector3 operator/(double scalar) const;

    Vector3& operator+=(const Vector3& other);
    Vector3& operator*=(double scalar);

    // Utilities
    double Length() const;
    Vector3 Normalized() const;
};

Vector3 operator*(double scalar, const Vector3& v);