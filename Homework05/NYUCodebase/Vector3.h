#pragma once
#ifndef Vector3_h
#define Vector3_h
#include "Matrix.h"
#include <utility>

class Vector3 {
public:
	Vector3();
	Vector3(float x, float y, float z = 0.0f);

	operator std::pair<float, float>() const;

	float x;
	float y;
	float z;
};

Vector3 operator * (const Matrix& matrix, const Vector3& vector);

#endif