#include "Vector3.h"
#include <utility>

Vector3::Vector3() {};
Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {};

Vector3::operator std::pair<float, float>() const {
	return std::make_pair(x, y);
}

Vector3 operator * (const Matrix& matrix, const Vector3& vector) {
	Vector3 res;
	res.x = matrix.m[0][0] * vector.x + matrix.m[1][0] * vector.y + matrix.m[2][0] * vector.z + matrix.m[3][0];
	res.y = matrix.m[0][1] * vector.x + matrix.m[1][1] * vector.y + matrix.m[2][1] * vector.z + matrix.m[3][1];
	res.z = matrix.m[0][2] * vector.x + matrix.m[1][2] * vector.y + matrix.m[2][2] * vector.z + matrix.m[3][2];
	return res;
}