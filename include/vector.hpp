#pragma once

#include "constants.hpp"
#include <math.h>

/****************************************************************************/
class Vector {
public:
    float data[3];      // Just a simple 3D array!
    
    // For reference field access - i.e. "shortcuts"
    float& x;
    float& y;
    float& z;

    // Constructors
    Vector() : data({0.0f, 0.0f, 0.0f}), x(data[0]), y(data[1]), z(data[2]) {}

    Vector(float x, float y, float z) : data({x, y, z}), x(data[0]), y(data[1]), z(data[2]) {}
    Vector(int16_t x, int16_t y, int16_t z) : data({static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)}), x(data[0]), y(data[1]), z(data[2]) {}

    // Math Operations
    float magnitude() const {
        return data[0]*data[0] + data[1]*data[1] + data[2]*data[2]; 
    }

    Vector normalise() {
        float mag = magnitude();
        if (mag == 0.0f) {
            return Vector(0.0f, 0.0f, 0.0f);
        }
        return Vector(data[0] / mag, data[1] / mag, data[2] / mag);
    }

    Vector cross(const Vector& other) {
        return Vector(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }

    // Overloaded Operators
    Vector operator-(const Vector& other) {
        return Vector(
            data[0] - other.data[0],    // x
            data[1] - other.data[1],    // y
            data[2] - other.data[2]     // z
        );
    }

    // Self-Referencing Overloaded Operators
    Vector& operator-=(const Vector& other) {
        data[0] -= other.data[0];
        data[1] -= other.data[1];
        data[2] -= other.data[2];
        return *this;
    }

    Vector& operator+=(const Vector& other) {
        data[0] += other.data[0];
        data[1] += other.data[1];
        data[2] += other.data[2];
        return *this;
    }

    Vector& operator*=(const Vector& other) {
        data[0] *= other.data[0];
        data[1] *= other.data[1];
        data[2] *= other.data[2];
        return *this;
    }

    Vector& operator*=(float scalar) {
        data[0] *= scalar;
        data[1] *= scalar;
        data[2] *= scalar;
        return *this;
    }

    Vector& operator/=(float scalar) {
        // Guard against division by zero
        data[0] /= (scalar != 0.0f) ? scalar : 1.0f;
        data[1] /= (scalar != 0.0f) ? scalar : 1.0f;;
        data[2] /= (scalar != 0.0f) ? scalar : 1.0f;
        return *this;
    }

    // Output Stream Overload for easy printing
    void print(Stream& out) const {
        out.print(data[0], 4); out.print('\t');
        out.print(data[1], 4); out.print('\t');
        out.print(data[2], 4); out.print('\t');
    }
}; 