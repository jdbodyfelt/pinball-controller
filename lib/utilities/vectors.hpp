#ifndef VECTORS_HPP
#define VECTORS_HPP

#pragma once

#include <Arduino.h>
#include <math.h>
#include <type_traits>

/**********************************************************************************/
// Templated Vector class - T: type, N: dimension
template<typename T, size_t N>
class Vector {
    static_assert(N >= 1, "Vector dimension must be at least 1");
    static_assert(std::is_arithmetic<T>::value, "Vector must use arithmetic types");

private:
    T data[N];

public:
    // Constructors
    Vector() { 
        for(size_t i = 0; i < N; ++i) data[i] = T(0); 
    }
    
    // Single value constructor (all components same value)
    explicit Vector(T value) {
        for(size_t i = 0; i < N; ++i) data[i] = value;
    }
    
    // Variadic constructor for initializing all components
    template<typename... Args>
    Vector(Args... args) : data{static_cast<T>(args)...} {
        static_assert(sizeof...(Args) == N, "Number of arguments must match vector dimension");
    }
    
    // Copy constructor
    Vector(const Vector& other) {
        for(size_t i = 0; i < N; ++i) data[i] = other.data[i];
    }

    // Array subscript operator
    T& operator[](size_t index) { 
        return data[index]; 
    }
    
    const T& operator[](size_t index) const { 
        return data[index]; 
    }

    // Assignment operator
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            for(size_t i = 0; i < N; ++i) data[i] = other.data[i];
        }
        return *this;
    }

    // += Operator
    Vector& operator+=(const Vector& other) {
        for(size_t i = 0; i < N; ++i) data[i] += other.data[i];
        return *this;
    }

    // + Operator
    Vector operator+(const Vector& other) const {
        Vector result = *this;
        result += other;
        return result;
    }

    // -= Operator
    Vector& operator-=(const Vector& other) {
        for(size_t i = 0; i < N; ++i) data[i] -= other.data[i];
        return *this;
    }

    // - Operator
    Vector operator-(const Vector& other) const {
        Vector result = *this;
        result -= other;
        return result;
    }

    // *= Scalar Operator
    Vector& operator*=(T scalar) {
        for(size_t i = 0; i < N; ++i) data[i] *= scalar;
        return *this;
    }

    // * Scalar Operators
    Vector operator*(T scalar) const {
        Vector result = *this;
        result *= scalar;
        return result;
    }

    friend Vector operator*(T scalar, const Vector& vec) {
        return vec * scalar;
    }

    // /= Scalar Operator
    Vector& operator/=(T scalar) {
        if (scalar != T(0)) {
            for(size_t i = 0; i < N; ++i) data[i] /= scalar;
        }
        return *this;
    }

    // / Scalar Operator
    Vector operator/(T scalar) const {
        Vector result = *this;
        result /= scalar;
        return result;
    }

    // Equality operators
    bool operator==(const Vector& other) const {
        for(size_t i = 0; i < N; ++i) {
            if(data[i] != other.data[i]) return false;
        }
        return true;
    }

    bool operator!=(const Vector& other) const {
        return !(*this == other);
    }

    // Magnitude (only for floating point types)
    template<typename U = T>
    typename std::enable_if<std::is_floating_point<U>::value, float>::type
    magnitude() const {
        float sum = 0;
        for(size_t i = 0; i < N; ++i) {
            sum += static_cast<float>(data[i]) * static_cast<float>(data[i]);
        }
        return sqrtf(sum);
    }

    // Magnitude squared (works for all arithmetic types)
    T magnitudeSquared() const {
        T sum = T(0);
        for(size_t i = 0; i < N; ++i) {
            sum += data[i] * data[i];
        }
        return sum;
    }

    // Dot product
    T dot(const Vector& other) const {
        T result = T(0);
        for(size_t i = 0; i < N; ++i) {
            result += data[i] * other.data[i];
        }
        return result;
    }

    // Normalize (only for floating point types)
    template<typename U = T>
    typename std::enable_if<std::is_floating_point<U>::value, Vector&>::type
    normalize() {
        float mag = magnitude();
        if (mag > 0) {
            *this *= (1.0f / mag);
        }
        return *this;
    }

    // Print to stream
    void print(Stream& stream, int precision = 2, String delim = "\t", bool newLine = true) const {
        for(size_t i = 0; i < N; ++i) {
            stream.print(data[i], precision);
            if(i < N - 1) stream.print(delim);
        }
        if(newLine) stream.println();
    }

    // Dimension access
    static constexpr size_t size() { return N; }

    // Valid Entry Checks
    bool hasNaN() const {
        for(size_t i = 0; i < N; ++i) {
            if (isnan(data[i])) return true;
        }
        return false;
    }
    
    bool isValid() const {
        for(size_t i = 0; i < N; ++i) {
            if (isnan(data[i]) || isinf(data[i])) return false;
        }
        return true;
    }
};
/**********************************************************************************/
// Specialized cross product for 3D vectors
template<typename T>
Vector<T, 3> cross(const Vector<T, 3>& a, const Vector<T, 3>& b) {
    return Vector<T, 3>(
        a[1] * b[2] - a[2] * b[1],
        a[2] * b[0] - a[0] * b[2],
        a[0] * b[1] - a[1] * b[0]
    );
}
/**********************************************************************************/
// Tilt projection: Convert 3D vector to 2D tilt vector (pitch, roll)
template<typename T>
Vector<T, 2> tiltProjection(const Vector<T, 3>& vec) {
    // Returns (pitch, roll) in degrees
    // Pitch: rotation around Y-axis (X-Z plane)
    // Roll: rotation around X-axis (Y-Z plane)
    static_assert(std::is_floating_point<T>::value, "Tilt projection requires floating point types");
    
    T pitch = atan2(-vec[0], sqrt(vec[1]*vec[1] + vec[2]*vec[2])) * RAD_TO_DEG;
    T roll = atan2(vec[1], vec[2]) * RAD_TO_DEG;
    
    return Vector<T, 2>(pitch, roll);
}
/**********************************************************************************/
// Common type aliases
using Vec2f = Vector<float, 2>;
using Vec3f = Vector<float, 3>;
using Vec2i = Vector<int, 2>;
using Vec3i = Vector<int, 3>;
using Vec2d = Vector<double, 2>;
using Vec3d = Vector<double, 3>;
/**********************************************************************************/
#endif