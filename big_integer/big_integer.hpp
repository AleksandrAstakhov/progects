#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string>

class BigInt {
 public:
  BigInt();
  explicit BigInt(const std::string& value);
  explicit BigInt(const int64_t& value);
  BigInt(const BigInt& other);
  BigInt& operator=(const BigInt& other);
  BigInt& operator+=(const BigInt& other);
  BigInt operator+(const BigInt& other) const;
  BigInt& operator++();
  BigInt operator++(int);
  BigInt& operator-=(const BigInt& other);
  BigInt operator-(const BigInt& other) const;
  BigInt operator-() const;
  BigInt& operator--();
  BigInt operator--(int);
  BigInt& operator*=(const BigInt& other);
  BigInt operator*(const BigInt& other) const;
  BigInt& operator/=(const BigInt& other);
  BigInt operator/(const BigInt& other) const;
  BigInt& operator%=(const BigInt& other);
  BigInt operator%(const BigInt& other) const;
  bool operator<(const BigInt& other) const;
  bool operator<=(const BigInt& other) const;
  bool operator>(const BigInt& other) const;
  bool operator>=(const BigInt& other) const;
  bool operator==(const BigInt& other) const;
  bool operator!=(const BigInt& other) const;

  friend std::ostream& operator<<(std::ostream& out, const BigInt& number);

 private:
  bool sgn_;
  std::vector<int64_t> numbers_;
  const int64_t kBase = 1e9;

  void MinusEqual(const BigInt& second);  // with no-same sgn_
  void PlusEqual(const BigInt& second);   // with same sgn_
  static BigInt MultiplyByBase(int64_t number, int64_t power);
  bool AbsComparison(const BigInt& first, const BigInt& second) const;
  BigInt& DevisionByTwo();
  BigInt Modul() const;
};

std::istream& operator>>(std::istream& input, BigInt& number);
