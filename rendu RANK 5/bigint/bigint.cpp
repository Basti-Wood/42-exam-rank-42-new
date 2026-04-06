#include "bigint.hpp"

bigint::bigint() : value("0") {}

bigint::bigint(unsigned int val) : value(std::to_string(val)) {}
bigint::bigint(std::string val) : value(val) {}

bigint::bigint(const bigint& other) : value(other.value) {}

bigint& bigint::operator=(const bigint& other) {
	if (this != &other) {
		value = other.value;
	}
	return *this;
}

bigint::~bigint() {}

//helper

static std::string reverse(const std::string& str) {
	std::string reversed(str.rbegin(), str.rend());
	return reversed;
}	


static std::string addition(std::string a, std::string b)
{
	std::string tempa = reverse(a);
	std::string tempb = reverse(b);
	std::string result;
	int carry = 0;
	int maxLength = std::max(tempa.length(), tempb.length());

	for (int i = 0; i < maxLength; ++i) {
		int digitA = (i < tempa.length()) ? tempa[i] - '0' : 0;
		int digitB = (i < tempb.length()) ? tempb[i] - '0' : 0;
		int sum = digitA + digitB + carry;
		result += (sum % 10) + '0';
		carry = sum / 10;
	}
	if (carry) {
		result += carry + '0';
	}
	return reverse(result);
}

static std::string strip_leading_zeros(const std::string& s) {
	size_t i = 0;
	while (i < s.size() && s[i] == '0') {
		++i;
	}
	return (i == s.size()) ? "0" : s.substr(i);
}

//operators

bigint bigint::operator+(const bigint& other) const {
	return bigint(addition(this->value, other.value));
}

bool bigint::operator<(const bigint& other) const
{
	std::string left = strip_leading_zeros(value);
	std::string right = strip_leading_zeros(other.value);

	if (left.length() != right.length()) {
		return left.length() < right.length();
	}
	return left < right;
}

bool bigint::operator>(const bigint& other) const
{
	std::string left = strip_leading_zeros(value);
	std::string right = strip_leading_zeros(other.value);

	if (left.length() != right.length()) {
		return left.length() > right.length();
	}
	return left > right;
}

bool bigint::operator==(const bigint& other) const
{
	std::string left = strip_leading_zeros(value);
	std::string right = strip_leading_zeros(other.value);
	return left == right;
}

bool bigint::operator!=(const bigint& other) const
{
	return !(*this == other);
}

bool bigint::operator<=(const bigint& other) const
{
	return (*this < other) || (*this == other);
}

bool bigint::operator>=(const bigint& other) const
{
	return (*this > other) || (*this == other);
}


bigint bigint::operator>>(int shift) const
{
	std::string shiftedValue = value.substr(0, value.length() - shift);
	return bigint(shiftedValue.empty() ? "0" : shiftedValue);
}

bigint bigint::operator<<(int shift) const
{
	std::string shiftedValue = value + std::string(shift, '0');
	return bigint(shiftedValue);
}



//printing
std::ostream& operator<<(std::ostream& os, const bigint& num) {
	os << num.value;
	return os;
}