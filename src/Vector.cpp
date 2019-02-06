#include "Vector.h"
#include <cmath>

double Vec3::length()
{
	return sqrt(x()*x() + y()*y() + z()*z());
}

Vec3 Vec3::normalize()
{
	double length = this->length();
	return Vec3(x() / length, y() / length, z() / length);
}

Vec3 Vec3::operator*(Vec3 other)
{
	return Vec3(x() * other.x(), y() * other.y(), z() * other.z());
}

Vec3 Vec3::operator*(double other)
{
	return Vec3(x()*other,y()*other,z()*other);
}

Vec3 Vec3::operator-(Vec3 other)
{
	return Vec3(x() - other.x(), y() - other.y(), z() - other.z());
}

Vec3 Vec3::operator-(double other)
{
	return Vec3(x() -other,y() -other,z() -other);
}

Vec3 Vec3::operator+(Vec3 other)
{
	return Vec3(x() + other.x(), y() + other.y(), z() + other.z());
}

Vec3 Vec3::operator+(double other)
{
	return Vec3(x() +other,y() +other,z() +other);
}

Vec3 Vec3::operator/(Vec3 other)
{
	return Vec3(x() / other.x(), y() / other.y(), z() / other.z());
}

Vec3 Vec3::operator/(double other)
{
	return operator*(1/other);
}
