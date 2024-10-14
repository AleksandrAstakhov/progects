#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

template <size_t N, size_t M, typename T = int64_t>
class Matrix {
 public:
  Matrix() : matrix_(std::vector<std::vector<T>>(N, std::vector<T>(M))) {}
  Matrix(const std::vector<std::vector<T>>& mat) : matrix_(mat) {}
  Matrix(const T& elem) {
    for (size_t i = 0; i < N; ++i) {
      matrix_.emplace_back(std::vector<T>(M, elem));
    }
  }
  Matrix(const Matrix& mat) : matrix_(mat.matrix_) {}
  Matrix& operator=(const Matrix& mat) {
    matrix_ = mat.matrix_;
    return *this;
  }
  Matrix& operator+=(const Matrix& mat) {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        matrix_[i][j] += mat.matrix_[i][j];
      }
    }
    return *this;
  }
  Matrix operator+(const Matrix& mat) const {
    Matrix copy = *this;
    copy += mat;
    return copy;
  }
  Matrix& operator-=(const Matrix& mat) {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        matrix_[i][j] -= mat.matrix_[i][j];
      }
    }
    return *this;
  }
  Matrix operator-(const Matrix& mat) const {
    Matrix copy = *this;
    return copy -= mat;
  }
  Matrix& operator*=(const T& elem) {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        matrix_[i][j] *= elem;
      }
    }
    return *this;
  }

  template <size_t M1>
  Matrix<N, M1, T> operator*(const Matrix<M, M1, T>& mat) const {
    std::vector<std::vector<T>> result(N);
    for (size_t i = 0; i < N; ++i) {
      for (size_t m1 = 0; m1 < M1; ++m1) {
        T sum = matrix_[i][0] * mat(0, m1);
        for (size_t j = 1; j < M; ++j) {
          sum += matrix_[i][j] * mat(j, m1);
        }
        result[i].push_back(sum);
      }
    }
    return result;
  }

  Matrix<M, N, T> Transposed() const {
    Matrix<M, N, T> result;
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        result(j, i) = matrix_[i][j];
      }
    }
    return result;
  }

  bool operator==(const Matrix& mat) const {
    for (size_t i = 0; i < N; ++i) {
      for (size_t j = 0; j < M; ++j) {
        if (matrix_[i][j] != mat.matrix_[i][j]) {
          return false;
        }
      }
    }
    return true;
  }

  T& operator()(size_t i_elem, size_t j_elem) {
    return matrix_[i_elem][j_elem];
  }

  T operator()(size_t i_elem, size_t j_elem) const {
    return matrix_[i_elem][j_elem];
  }

  T Trace() const { return MyTrace(*this); }

 private:
  std::vector<std::vector<T>> matrix_;
};

template <size_t N, typename T = int64_t>
T MyTrace(const Matrix<N, N, T>& mat) {
  T res;
  for (size_t ind = 0; ind < N; ++ind) {
    res += mat(ind, ind);
  }
  return res;
}

template <size_t N, size_t M, typename T = int64_t>
Matrix<N, M, T> operator*(const T& elem, const Matrix<N, M, T>& mat) {
  Matrix copy = mat;
  return copy *= elem;
}

template <size_t N, size_t M, typename T = int64_t>
Matrix<N, M, T> operator*(const Matrix<N, M, T>& mat, const T& elem) {
  Matrix copy = mat;
  return copy *= elem;
}
