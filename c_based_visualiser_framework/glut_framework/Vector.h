/*
 * Copyright (c) 2015 The University of Manchester
 * based on work Copyright (c) 2010 Paul Solt, paulsolt@gmail.com
 * https://github.com/PaulSolt/GLUT-Object-Oriented-Framework
 * Originally released under the MIT License
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VECTOR_H
#define VECTOR_H

#include <string>
#include <math.h>
#include <string>
#include <stdio.h>

#include "Point.h"

/* Vector.cpp
 *
 * A templated vector object that has 3 components (x,y,z).
 *
 * Author: Paul Solt
 *	Updated: 2-1-09
 */

template <class T>
class Vector {

public: // Direct access to variables
    T x;
    T y;
    T z;

public:

    /* Create a vector with (0,0,0) */
    Vector();

    /* Create a vector with (x,y,z) */
    Vector( T x, T y, T z );

    Vector( const Vector &other );

    /* Creates a vector from two points */
    Vector( Point<T> p1, Point<T> p2 );

    /* Addition of vectors */
    Vector<T> operator+( const Vector<T> &other ) const;

    /* Subtraction of vectors */
    Vector<T> operator-( const Vector<T> &other ) const;

    /* Comparison operator */
    bool operator==( const Vector<T> &other ) const;

    /* Scale the vector */
    Vector<T> operator*( T scaleFactor ) const;

    Vector<T> operator/( T scaleFactor ) const;

    /* Vector Assignment */
    Vector<T> &operator=( const Vector<T> &other );

    /* Compute the dot product of two vectors */
    T dot( const Vector<T> &v1 ) const;

    /* Compute the dot product between a vector and a point */
    T dot( Point<T> p1 ) const;

    /* Computer the cross product of two vectors */
    Vector cross( const Vector<T> &v1 ) const;

    /* Compute the length of a vector */
    T length() const;

    /* Normalize the vector */
    void normalize();

    /* Reverse the direction of the vector */
    void reverse();

    Vector<T> reflect( const Vector<T> &source, const Vector<T> &normal );

    /* Transform (ROTATE) the vector by a given matrix */
    void transform( T matrix[4][4] );

    /* Set the vector to (x,y,z) components */
    void setVector( T x, T y, T z);

    /* Set the vector between points p1 and p2 */
    void setVector( const Point<T> &p1, const Point<T> &p2 );

    /* Set the vector given another vector */
    void setVector( const Vector<T> &other );

    /* Set the vector to (0,0,0) */
    void zero();

    /* Display a Vector */
    friend std::ostream & operator<<(std::ostream &os, const Vector<T> &v) {
        os << "Vector: (" << v.x << ", " << v.y << ", " << v.z << ")";
        return os;
    }

};

template <class T> Vector<T>::Vector()
: x(0), y(0), z(0) {
}

template <class T> Vector<T>::Vector( T vx, T vy, T vz )
: x(vx), y(vy), z(vz) {
}

template <class T> Vector<T>::Vector( const Vector<T> &v )
: x(v.x), y(v.y), z(v.z) {
}

template <class T> Vector<T>::Vector( Point<T> p1, Point<T> p2 ) {
    setVector( (p1.x - p2.x), (p1.y - p2.y), (p1.z - p2.z) );
}

template <class T> Vector<T> Vector<T>::operator+( const Vector<T> &other ) const {
    Vector<T> result;
    result.x = this->x + other.x;
    result.y = this->y + other.y;
    result.z = this->z + other.z;
    return result;
}

template <class T> Vector<T> Vector<T>::operator-( const Vector<T> &other ) const {
    Vector<T> result;
    result.x = this->x - other.x;
    result.y = this->y - other.y;
    result.z = this->z - other.z;
    return result;
}

template <class T> bool Vector<T>::operator==( const Vector<T> &other ) const {
    return (this->x == other.x && this->y == other.y &&
        this->z == other.z);
}

template <class T> Vector<T> Vector<T>::operator*( T scaleFactor ) const {
    Vector<T> result( *this );
    result.x *= scaleFactor;
    result.y *= scaleFactor;
    result.z *= scaleFactor;
    return result;
}

template <class T> Vector<T> Vector<T>::operator/(T rhs) const {
    Vector<T> result(*this);
    result.x /= rhs;
    result.y /= rhs;
    result.z /= rhs;
    return result;
}

template <class T> Vector<T> &Vector<T>::operator=(const Vector<T> &other) {
    if (this == &other) {
        return *this;
    }
    setVector( other.x, other.y, other.z );
    return *this;
}

template <class T> T Vector<T>::dot(const Vector<T> &v1) const {
    return (x * v1.x + y * v1.y + z * v1.z);
}

template <class T> T Vector<T>::dot(Point<T> p1) const {
    return (x * p1.x + y * p1.y + z * p1.z);
}

template <class T> Vector<T> Vector<T>::cross(const Vector<T> &v1) const {
    Vector<T> result;
    result.x = ( y * v1.z - z * v1.y );
    result.y = -( x * v1.z - z * v1.x );
    result.z = ( x * v1.y - y * v1.x );
    return result;
}

template <class T> T Vector<T>::length() const {
    return sqrt( x*x + y*y + z*z );
}

template <class T> void Vector<T>::normalize() {
    T length = this->length(); /* Compute the magnitude */

    if (length != 0) { /* Prevent divide by zero */
        x /= length;
        y /= length;
        z /= length;
    } else {
        //printf( "VECTOR DIVIDE BY ZERO ERROR\n" );
    }
}

template <class T> void Vector<T>::reverse() {
    x = -x;
    y = -y;
    z = -z;
}

template <class T> Vector<T> Vector<T>::reflect( const Vector<T> &source,
        const Vector<T> &normal ) {
    Vector<T> result = (normal * (2.0 * source.dot( normal ) ) );
    result = source - result;
    result.normalize();
    return result;
}

template <class T> void Vector<T>::transform( T matrix[4][4] ) {
    T value[] = { 0, 0, 0 };
    value[0] = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z;
    value[1] = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z;
    value[2] = matrix[2][0] * x + matrix[2][1] * y + matrix[2][2] * z;
    x = value[0];
    y = value[1];
    z = value[2];
}

template <class T> void Vector<T>::setVector( T x, T y, T z ) {
    this->x = x;
    this->y = y;
    this->z = z;
}

template <class T> void Vector<T>::setVector( const Point<T> &p1, const Point<T> &p2 ) {
    setVector( (p1.x - p2.x), (p1.y - p2.y), (p1.z - p2.z), 1.0f );
}

template <class T> void Vector<T>::setVector( const Vector<T> &other ) {
    setVector( other.x, other.y, other.z );
}

template <class T> void Vector<T>::zero() {
    setVector(0.0, 0.0, 0.0);
}

#endif
