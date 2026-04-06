#include "vect2.hpp"

vect2::vect2(int x, int y) : x(x), y(y) {}

vect2::vect2() : x(0), y(0) {}

vect2::vect2(const vect2& other) : x(other.x), y(other.y) {}

vect2& vect2::operator=(const vect2& other)
{
	if (this != &other)
	{
		x = other.x;
		y = other.y;
	}
	return *this;
}

vect2::~vect2() {}

vect2 vect2::operator+(vect2 const& other) const
{
	return vect2(x + other.x, y + other.y);
}

vect2 vect2::operator-(vect2 const& other) const
{
	return vect2(x - other.x, y - other.y);
}


vect2 vect2::operator*(int scalar) const
{
	return vect2(x * scalar, y * scalar);
}


vect2& vect2::operator++()
{
	++x;
	++y;
	return *this;
}
vect2& vect2::operator--()
{
	--x;
	--y;
	return *this;
}


vect2 vect2::operator++(int)
{
	vect2 tmp(*this);
	x++;
	y++;
	return tmp;
}


vect2 vect2::operator--(int)
{
	vect2 tmp(*this);
	x--;
	y--;
	return tmp;
}

bool vect2::operator==(const vect2& other) const
{
	if (this->x == other.x && this->y == other.y)
		return true;
	return false;
}

bool vect2::operator!=(const vect2& other) const
{
	return !(*this == other);
}

vect2 vect2::operator-() const
{
	return vect2(-this->x, -this->y);
}
vect2& vect2::operator+=(const vect2& other)
{
	this->x += other.x;
	this->y += other.y;
	return *this;
}
vect2& vect2::operator-=(const vect2& other)
{
	this->x -= other.x;
	this->y -= other.y;
	return *this;
}
vect2& vect2::operator*=(int scalar)
{
	this->x *= scalar;
	this->y *= scalar;
	return *this;
}


int& vect2::operator[](size_t index)
{
	if (index == 0)
		return x;
	if (index == 1)
		return y;
	throw std::out_of_range("Index out of range");
}



const int vect2::operator[](size_t index) const
{
    if (index == 0) return x;
    if (index == 1) return y;
    throw std::out_of_range("Index out of range");
}

vect2 operator*(int scalar, const vect2& v)
{
	return v * scalar;
}

std::ostream& operator<<(std::ostream& os, const vect2& v)
{
	os << "{" << v.x << ", " << v.y << "}";
	return os;
}