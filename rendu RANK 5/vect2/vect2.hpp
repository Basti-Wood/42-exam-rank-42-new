#pragma once
#include <iostream>

class vect2
{
private:
	int x;
	int y;
public:
	vect2(int x, int y);
	vect2();
	vect2(const vect2& other);
	vect2& operator=(const vect2& other);
	~vect2();

	vect2 operator+(vect2 const& other) const;
	vect2 operator-(vect2 const& other) const;
	vect2 operator*(int scalar) const;
	vect2& operator++();
	vect2& operator--();
	vect2 operator++(int);
	vect2 operator--(int);
	bool operator==(const vect2& other) const;
	bool operator!=(const vect2& other) const;
	vect2 operator-() const;
	vect2& operator+=(const vect2& other);
	vect2& operator-=(const vect2& other);
	vect2& operator*=(int scalar);

	int& operator[](size_t index);
	const int operator[](size_t index) const;

	friend vect2 operator*(int scalar, const vect2& v);
	friend std::ostream& operator<<(std::ostream& os, const vect2& v);
};

