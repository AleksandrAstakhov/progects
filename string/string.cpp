#include "string.hpp"

size_t Minimum(size_t value_1, size_t value_2) {
  return value_1 <= value_2 ? value_1 : value_2;
}
void Exchange(size_t* val_1, size_t* val_2) {
  size_t tmp = *val_1;
  *val_1 = *val_2;
  *val_2 = tmp;
}
String::String() : size_(0), capacity_(0), string_(nullptr) {}
String::~String() { delete[] string_; }
String::String(size_t size, char character)
    : size_(size), capacity_(size_), string_(new char[capacity_ + 1]) {
  std::memset(string_, character, capacity_);
  string_[size_] = '\0';
}
String::String(const char* string) : size_(strlen(string)), capacity_(size_) {
  string_ = new char[capacity_ + 1];
  memcpy(string_, string, size_);
  string_[size_] = '\0';
}

void String::Resize(size_t new_size) {
  if (new_size >= capacity_) {
    char* copy_string = new char[new_size + 1];
    if (string_ != nullptr) {
      memcpy(copy_string, string_, size_);
      delete[] string_;
    }
    string_ = copy_string;
    capacity_ = new_size;
  }
  if (new_size > size_) {
    memset(string_ + size_, '\0', new_size - size_);
  }
  size_ = new_size;
  string_[new_size] = '\0';
}
void String::Reserve(size_t new_cap) {
  if (new_cap > capacity_) {
    size_t copy_size = size_;
    Resize(new_cap);
    size_ = copy_size;
    string_[size_] = '\0';
  }
}
String::String(const String& string)
    : size_(string.size_), capacity_(string.capacity_) {
  if (string.string_ != nullptr) {
    string_ = new char[capacity_ + 1];
    memcpy(string_, string.string_, string.size_ + 1);
  } else {
    string_ = nullptr;
  }
}
String& String::operator=(const String& string) {
  Reserve(string.capacity_);
  Resize(string.size_);
  if (string.string_ != nullptr) {
    memcpy(string_, string.string_, string.size_);
  }
  return *this;
}
void String::PushBack(char character) {
  if (++size_ >= capacity_) {
    Reserve(capacity_ == 0 ? 1 : 2 * capacity_);
  }
  string_[size_ - 1] = character;
  string_[size_] = '\0';
}
void String::PopBack() {
  if (size_ != 0) {
    string_[--size_] = '\0';
  }
}
void String::Clear() {
  size_ = 0;
  if (string_ != nullptr) {
    string_[size_] = '\0';
  }
}
void String::Resize(size_t new_size, char character) {
  if (size_ < new_size) {
    Reserve(new_size);
    memset(&string_[size_], character, new_size - size_);
  }
  size_ = new_size;
  if (string_ != nullptr) {
    string_[new_size] = '\0';
  }
}
void String::ShrinkToFit() {
  if (capacity_ > size_) {
    char* tmp_string = new char[size_ + 1];
    memcpy(string_, tmp_string, size_ + 1);
    delete[] string_;
    string_ = tmp_string;
  }
  capacity_ = size_;
}
void String::Swap(String& other) {
  char* tmp = other.string_;
  other.string_ = string_;
  string_ = tmp;
  Exchange(&size_, &other.size_);
  Exchange(&capacity_, &other.capacity_);
}

char& String::operator[](size_t index) { return this->string_[index]; }
const char& String::operator[](size_t index) const {
  return this->string_[index];
}
char& String::Front() { return this->string_[0]; }
char String::Front() const { return this->string_[0]; }
char& String::Back() { return this->string_[size_ - 1]; }
char String::Back() const { return this->string_[size_ - 1]; }
bool String::Empty() const { return size_ == 0; }
size_t String::Size() const { return size_; }
size_t String::Capacity() const { return capacity_; }
const char* String::Data() const { return string_; }
char* String::Data() { return string_; }

String& String::operator+=(const String& string) {
  for (size_t i = 0; i < string.size_; ++i) {
    PushBack(string.string_[i]);
  }
  return *this;
}
String String::operator+(const String& string) const {
  String copy = *this;
  copy += string;
  return copy;
}
String String::operator*(size_t n) const {
  String copy = *this;
  return copy *= n;
}
String& String::operator*=(size_t n) {
  if (n != 0) {
    Reserve(size_ * n);
    for (size_t i = size_; i < capacity_; i += size_) {
      memcpy(string_ + i, string_, size_);
    }
    size_ = capacity_;
    string_[size_] = '\0';
    return *this;
  }
  Resize(0);
  return *this;
}
bool String::operator<(const String& string_2) const {
  if (string_ == nullptr) {
    return string_2.string_ != nullptr ? 0 < string_2.size_ : false;
  }
  if (string_2.string_ == nullptr) {
    return false;
  }
  size_t minimun = Minimum(this->Size(), string_2.Size());
  for (size_t i = 0; i < minimun; ++i) {
    if (string_[i] == string_2.string_[i]) {
      continue;
    }
    return string_[i] < string_2.string_[i];
  }
  return Size() < string_2.Size();
}

std::vector<String> String::Split(const String& delim) const {
  std::vector<String> list;
  String sub_string;
  for (size_t i = 0; i < size_; ++i) {
    for (size_t shift = 0; shift < delim.size_; ++shift) {
      if (string_[i + shift] != delim[shift]) {
        sub_string.PushBack(string_[i]);
        break;
      }
      if (shift == delim.size_ - 1) {
        i += shift;
        list.push_back(sub_string);
        sub_string.Clear();
      }
    }
  }
  list.push_back(sub_string);
  return list;
}
String String::Join(const std::vector<String>& string) const {
  String result;
  size_t num_of_elemnt = 1;
  for (auto character : string) {
    if (num_of_elemnt != string.size()) {
      result += (character + *this);
      ++num_of_elemnt;
    } else {
      result += character;
    }
  }
  return result;
}

bool operator>(const String& string_1, const String& string_2) {
  return string_2 < string_1;
}
bool operator<=(const String& string_1, const String& string_2) {
  return !(string_1 > string_2);
}
bool operator>=(const String& string_1, const String& string_2) {
  return !(string_1 < string_2);
}
bool operator==(const String& string_1, const String& string_2) {
  return !(string_1 < string_2 || string_1 > string_2);
}
bool operator!=(const String& string_1, const String& string_2) {
  return string_1 < string_2 || string_1 > string_2;
}

std::ostream& operator<<(std::ostream& out, const String& string) {
  for (size_t i = 0; i < string.size_; ++i) {
    out << string.string_[i];
  }
  return out;
}

std::istream& operator>>(std::istream& input, String& string) {
  string.Clear();
  char character;
  while ((input.get(character)) && !(input.eof())) {
    string.PushBack(character);
  }
  return input;
}
