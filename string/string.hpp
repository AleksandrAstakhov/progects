#pragma once

#include <cstring>
#include <initializer_list>
#include <iostream>
#include <vector>

class String {
 public:
  String();
  ~String();
  explicit String(size_t size, char character);
  explicit String(const char* string);

  void Resize(size_t new_size);
  void Reserve(size_t new_cap);
  String(const String& string);
  void PushBack(char character);
  void PopBack();
  void Clear();
  void Resize(size_t new_size, char character);
  void ShrinkToFit();
  void Swap(String& other);
  char& Front();
  char Front() const;
  char& Back();
  char Back() const;
  bool Empty() const;
  size_t Size() const;
  size_t Capacity() const;
  const char* Data() const;
  char* Data();

  char& operator[](size_t index);
  const char& operator[](size_t index) const;
  String& operator=(const String& string);
  String& operator+=(const String& string);
  String operator+(const String& string) const;
  String operator*(size_t n) const;
  String& operator*=(size_t n);
  bool operator<(const String& string_2) const;

  friend std::ostream& operator<<(std::ostream& out, const String& string);

  std::vector<String> Split(const String& delim = " ") const;
  String Join(const std::vector<String>& string) const;

 private:
  size_t size_;
  size_t capacity_;
  char* string_;
};

bool operator>(const String& string_1, const String& string_2);
bool operator<=(const String& string_1, const String& string_2);
bool operator>=(const String& string_1, const String& string_2);
bool operator==(const String& string_1, const String& string_2);
bool operator!=(const String& string_1, const String& string_2);

std::ostream& operator<<(std::ostream& out, const String& string);
std::istream& operator>>(std::istream& input, String& string);
