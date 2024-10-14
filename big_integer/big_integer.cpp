#include "big_integer.hpp"

BigInt::BigInt() : sgn_(false), numbers_({0}) {}

BigInt::BigInt(const std::string& value) {
  value[0] == '-' && value[1] != '0' ? sgn_ = true : sgn_ = false;
  const int64_t kConst8 = 8;
  const int64_t kConst9 = 9;
  int64_t sgn = static_cast<int64_t>(sgn_);
  int64_t index = value.size() - 1;
  for (; index - kConst8 >= sgn; index -= kConst9) {
    numbers_.push_back(stoi(value.substr(index - kConst8, kConst9)));
  }
  if (index - sgn >= 0) {
    numbers_.push_back(stoi(value.substr(sgn, index - sgn + 1)));
  }
}

BigInt::BigInt(const int64_t& value) : BigInt(std::to_string(value)) {}

BigInt::BigInt(const BigInt& other)
    : sgn_(other.sgn_), numbers_(other.numbers_) {}

BigInt& BigInt::operator=(const BigInt& other) {
  sgn_ = other.sgn_;
  numbers_ = other.numbers_;
  return *this;
}

BigInt& BigInt::operator+=(const BigInt& other) {
  if (sgn_ == other.sgn_) {
    int64_t add_part = 0;
    size_t min = std::min(numbers_.size(), other.numbers_.size());
    size_t index = 0;
    for (; index < min; ++index) {
      add_part =
          (numbers_[index] += (other.numbers_[index] + add_part)) / kBase;
      numbers_[index] %= kBase;
    }
    for (; index < other.numbers_.size(); ++index) {
      numbers_.push_back(other.numbers_[index] + add_part);
      add_part = numbers_[index] / kBase;
      numbers_[index] %= kBase;
    }
    for (; index < numbers_.size(); ++index) {
      numbers_[index] += add_part;
      add_part = numbers_[index] / kBase;
      numbers_[index] %= kBase;
    }
    if (add_part != 0) {
      numbers_.push_back(add_part);
    }
  } else {
    sgn_ = !(sgn_);
    *this -= other;
    sgn_ = !(sgn_);
  }
  return *this;
}

BigInt BigInt::operator+(const BigInt& other) const {
  BigInt result = *this;
  return result += other;
}

BigInt& BigInt::operator++() {
  *this += 1;
  return *this;
}

BigInt BigInt::operator++(int) {
  BigInt copy = *this;
  ++*this;
  return copy;
}

BigInt& BigInt::operator-=(const BigInt& other) {
  if (sgn_ == !(other.sgn_)) {
    sgn_ = !(sgn_);
    *this += other;
    sgn_ = !(sgn_);
    return *this;
  }
  if (AbsComparison(other, *this)) {
    sgn_ = *this < other;
    MinusEqual(other);
  } else {
    *this = other - *this;
    sgn_ = !(sgn_);
  }
  return *this;
}

BigInt BigInt::operator-(const BigInt& other) const {
  BigInt copy = *this;
  return copy -= other;
}

BigInt BigInt::operator-() const {
  BigInt copy = *this;
  copy.sgn_ = !copy.sgn_;
  return copy;
}

BigInt& BigInt::operator--() {
  *this -= 1;
  return *this;
}

BigInt BigInt::operator--(int) {
  BigInt copy = *this;
  --*this;
  return copy;
}

BigInt& BigInt::operator*=(const BigInt& other) {
  BigInt result("0");
  int64_t add_part = 0;
  for (size_t j = 0; j < numbers_.size(); ++j) {
    for (size_t i = 0; i < other.numbers_.size(); ++i) {
      result += MultiplyByBase(numbers_[j] * other.numbers_[i], i + j);
    }
  }
  for (int i = result.numbers_.size() - 1; i > 0; --i) {
    if (result.numbers_[i] == 0) {
      result.numbers_.pop_back();
    }
  }
  bool tmp_sgn =
      static_cast<bool>(static_cast<int>(sgn_) ^ static_cast<int>(other.sgn_));
  *this = result;
  sgn_ = tmp_sgn;
  return *this;
}

BigInt BigInt::operator*(const BigInt& other) const {
  BigInt copy = *this;
  return copy *= other;
}

BigInt& BigInt::operator/=(const BigInt& other) {
  if (AbsComparison(*this, other)) {
    *this = 0;
    return *this;
  }
  BigInt last = (*this + 1).Modul();
  BigInt first = 1;
  while (last > first) {
    BigInt mid = (first + last).DevisionByTwo();
    if (mid * other.Modul() < Modul()) {
      first = mid + 1;
    } else {
      last = mid;
    }
  }
  bool tmp_sgn =
      static_cast<bool>(static_cast<int>(sgn_) ^ static_cast<int>(other.sgn_));
  if (last * other == *this) {
    *this = last;
  } else {
    *this = last - 1;
  }
  sgn_ = tmp_sgn;
  return *this;
}

BigInt BigInt::operator/(const BigInt& other) const {
  BigInt copy = *this;
  return copy /= other;
}

BigInt& BigInt::operator%=(const BigInt& other) {
  *this -= ((*this / other) * other);
  return *this;
}

BigInt BigInt::operator%(const BigInt& other) const {
  BigInt copy = *this;
  return copy %= other;
}

bool BigInt::operator<(const BigInt& other) const {
  if (numbers_.size() < other.numbers_.size()) {
    return static_cast<bool>(static_cast<int>(true) -
                             static_cast<int>(!(sgn_ == other.sgn_)) -
                             static_cast<int>(sgn_));
  }
  if (numbers_.size() > other.numbers_.size()) {
    return static_cast<bool>(static_cast<int>(true) -
                             static_cast<int>(sgn_ == other.sgn_) +
                             static_cast<int>(sgn_));
  }
  for (int16_t i = numbers_.size() - 1; i >= 0; --i) {
    if (numbers_[i] < other.numbers_[i]) {
      return static_cast<bool>(static_cast<int>(true) -
                               static_cast<int>(!(sgn_ == other.sgn_)) -
                               static_cast<int>(sgn_));
    }
    if (numbers_[i] > other.numbers_[i]) {
      return static_cast<bool>(static_cast<int>(true) -
                               static_cast<int>(sgn_ == other.sgn_) +
                               static_cast<int>(sgn_));
    }
  }
  if (numbers_[0] == 0 && numbers_.size() == 1) {
    return false;
  }
  return sgn_ && !other.sgn_;
}

bool BigInt::operator>(const BigInt& other) const { return other < *this; }

bool BigInt::operator<=(const BigInt& other) const { return !(*this > other); }

bool BigInt::operator>=(const BigInt& other) const { return !(*this < other); }

bool BigInt::operator!=(const BigInt& other) const { return !(*this == other); }

bool BigInt::operator==(const BigInt& other) const {
  return *this <= other && *this >= other;
}

std::ostream& operator<<(std::ostream& out, const BigInt& number) {
  std::string result;
  const int64_t kConst9 = 9;
  int64_t size = static_cast<int64_t>(number.numbers_.size());
  if (number.sgn_) {
    result += "-";
  }
  while (number.numbers_[size - 1] == 0 && size != 1) {
    --size;
  }
  for (int64_t i = size - 1; i >= 0; --i) {
    if (i != size - 1 && std::to_string(number.numbers_[i]).size() < kConst9) {
      for (size_t j = 0;
           j < kConst9 - std::to_string(number.numbers_[i]).size(); ++j) {
        result += "0";
      }
      if (number.numbers_[i] != 0) {
        result += std::to_string(number.numbers_[i]);
      }
      continue;
    }
    result += std::to_string(number.numbers_[i]);
  }
  out << result;
  return out;
}

std::istream& operator>>(std::istream& input, BigInt& number) {
  std::string str;
  input >> str;
  number = BigInt(str);
  return input;
}

void BigInt::MinusEqual(const BigInt& second) {  // with no-same sgn_
  int64_t tmp_neg_part = 0;
  int64_t neg_part = 0;
  size_t index = 0;
  for (; index < second.numbers_.size(); ++index) {
    numbers_[index] -= (second.numbers_[index] + neg_part);
    if (numbers_[index] < 0) {
      ++tmp_neg_part;
      numbers_[index] += kBase;
    }
    neg_part = tmp_neg_part;
    tmp_neg_part = 0;
  }
  while (neg_part != 0) {
    (numbers_[index] -= neg_part) < 0 ? neg_part = 1 : neg_part = 0;
    ++index;
  }
  while ((numbers_[index - 1] == 0) && (index != 1)) {
    numbers_.pop_back();
    --index;
  }
}

BigInt BigInt::MultiplyByBase(int64_t number, int64_t power) {
  BigInt num(number);
  for (int64_t i = 0; i < power; ++i) {
    num.numbers_.insert(num.numbers_.begin(), 0);
  }
  return num;
}

bool BigInt::AbsComparison(const BigInt& first, const BigInt& second) const {
  if (first.numbers_.size() < second.numbers_.size()) {
    return true;
  }
  if (first.numbers_.size() > second.numbers_.size()) {
    return false;
  }
  for (int16_t i = numbers_.size() - 1; i >= 0; --i) {
    if (first.numbers_[i] < second.numbers_[i]) {
      return true;
    }
    if (first.numbers_[i] > second.numbers_[i]) {
      return false;
    }
  }
  return true;
}

BigInt& BigInt::DevisionByTwo() {
  BigInt five("500000000");
  BigInt tmp = numbers_[0] / 2;
  numbers_.erase(numbers_.begin());
  (*this *= five) += tmp;
  return *this;
}

BigInt BigInt::Modul() const {
  BigInt copy = *this;
  copy.sgn_ = false;
  return copy;
}
