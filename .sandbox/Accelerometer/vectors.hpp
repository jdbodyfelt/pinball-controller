#pragma once 

#include <Stream.h>             // Arduino Stream base class
#include <etl/vector.h>
#include <math.h>

#include "ButterworthFilter.hpp"

/***************************************************************************/
using xyCoords = etl::vector<float, 2>; 

using vec3bw = etl::vector<ButterworthFilter, 3>; 

/***************************************************************************/
class Vec3f {
private:
    etl::vector<float, 3> _data; 

public: 
    float& x;       // Public reference members
    float& y; 
    float& z; 

    // Constructors
    Vec3f(float x_val=0, float y_val=0, float z_val=0): 
        _data{x_val,y_val,z_val}, 
        x(_data[0]), y(_data[1]), z(_data[2])
    {}
    // Copy Constructor
    Vec3f(const Vec3f& other)
        : _data{other._data[0], other._data[1], other._data[2]}
        , x(_data[0])
        , y(_data[1]) 
        , z(_data[2])
    {}
    // Assignment Operator
    Vec3f& operator=(const Vec3f& other) {
        x = other.x;
        y = other.y;
        z = other.z;
        return *this;
    }
    // += Operator
    Vec3f& operator+=(const Vec3f& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    // + Operator (non-member friend)
    friend Vec3f operator+(Vec3f lhs, const Vec3f& rhs) {
        lhs += rhs;
        return lhs;
    }
    // -= Operator
    Vec3f& operator-=(const Vec3f& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
    // - Operator (non-member friend)
    friend Vec3f operator-(Vec3f lhs, const Vec3f& rhs) {
        lhs -= rhs;
        return lhs;
    }
    // *= Scalar Operator
    Vec3f& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }
    // * Scalar Operators (non-member friends)
    friend Vec3f operator*(Vec3f vec, float scalar) {
        vec *= scalar;
        return vec;
    }
    friend Vec3f operator*(float scalar, Vec3f vec) {
        vec *= scalar;
        return vec;
    }
    // /= Scalar Operator
    Vec3f& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }
    // / Scalar Operator (non-member friend)
    friend Vec3f operator/(Vec3f vec, float scalar) {
        vec /= scalar;
        return vec;
    }
    // Magnitude
    float magnitude() const {
        return sqrt(x*x + y*y + z*z);
    }
    // Normalize
    Vec3f& normalize() {
        float mag = magnitude();
        if (mag > 0) {
            *this /= mag;
        }
        return *this;
    }
    // Apply Butterworth filtering
    Vec3f& filter(vec3bw& filts){
        x = filts[0].process(x);
        y = filts[1].process(y); 
        z = filts[2].process(z);
        return *this; 
    }
    // Dot product
    float dot(const Vec3f& other) const {
        return x*other.x + y*other.y + z*other.z;
    }
    // Cross product using ^ operator
    Vec3f cross(const Vec3f& other) const {
        return Vec3f(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
    // Convert to etl::vector
    operator etl::vector<float, 3>() const {
        return etl::vector<float, 3>{x, y, z};
    }
    // Euler Angles
    float pitch() const {
        float denom = sqrt(x*x+z*z);
        float q = atan2(-y, denom);
        return degrees(q);
    }
    float roll() const {
        float denom = sqrt(y*y+z*z);
        float q = atan2(x, x*x + z*z);
        return degrees(q);
    }
    // Print to a stream device
    void print(Stream& stream=Serial, int precision=4, String delim="\t", bool newLine=true) const { 
        stream.print(x, precision);
        stream.print(delim); 
        stream.print(y, precision);
        stream.print(delim); 
        stream.print(z, precision);
        if(newLine){ 
            stream.print("\n");
        }
    }
};
/***************************************************************************/