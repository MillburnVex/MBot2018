class Vec3 {
private:
	double xv;
	double yv;
	double zv;
public:
	Vec3(double _x, double _y, double _z) : xv(_x), yv(_y), zv(_z) {}
	Vec3() : Vec3(0, 0, 0) {}

	double x() { return xv; }
	double y() { return yv; }
	double z() { return zv; }

	double length();
	Vec3 normalize();

	Vec3 operator*(Vec3 other);
	Vec3 operator*(double other);

	Vec3 operator-(Vec3 other);
	Vec3 operator-(double other);

	Vec3 operator+(Vec3 other);
	Vec3 operator+(double other);

	Vec3 operator/(Vec3 other);
	Vec3 operator/(double other);

};