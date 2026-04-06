#pragma once
#include <string>

class bigint
{
private:
	std::string value;
public:
	bigint();
	bigint(unsigned int val);
	bigint(std::string val);
	bigint(const bigint& other);
	bigint& operator=(const bigint& other);
	~bigint();

	//operators
	bigint operator+(const bigint& other) const;
	bool operator<(const bigint& other) const;
	bool operator>(const bigint& other) const;
	bool operator==(const bigint& other) const;
	bool operator!=(const bigint& other) const;
	bool operator<=(const bigint& other) const;
	bool operator>=(const bigint& other) const;
	bigint operator>>(int shift) const;
	bigint operator<<(int shift) const;

	friend std::ostream& operator<<(std::ostream& os, const bigint& num);
};

