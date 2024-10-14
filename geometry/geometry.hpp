#pragma once

#include <stdint.h>

class Point;
class Segment;

class Vector {
 public:
  Vector();
  Vector(int64_t coord_x, int64_t coord_y);
  int64_t operator*(const Vector& vec) const;
  int64_t operator^(const Vector& vec) const;
  Vector& operator+=(const Vector& vec);
  Vector operator+(const Vector& vec) const;
  Vector& operator-=(const Vector& vec);
  Vector operator-(const Vector& vec) const;
  Vector& operator*=(int64_t num);
  Vector& operator-();
  bool operator==(const Vector& vec) const;
  int64_t GetX() const;
  int64_t GetY() const;
  int64_t ModulSquare() const;
  bool CollinearWith(const Vector& vec) const;
  bool CollinearSameDirect(const Vector& vec) const;

 private:
  int64_t x_;
  int64_t y_;
};

Vector operator*(int64_t num, const Vector& vec);
Vector operator*(const Vector& vec, int64_t num);

class IShape {
 public:
  virtual void Move(const Vector& vec) = 0;
  virtual bool ContainsPoint(const Point& point) const = 0;
  virtual bool CrossSegment(const Segment& segm) const = 0;
  virtual IShape* Clone() const = 0;
  virtual ~IShape() = default;
};

class Point : public IShape {
 public:
  Point();
  ~Point() override = default;
  Point(int64_t coord_x, int64_t coord_y);
  Point(const Point& point);
  bool operator==(const Point& point) const;
  Vector operator-(const Point& point) const;
  int64_t GetX() const;
  int64_t GetY() const;
  void Move(const Vector& vec) override;
  bool ContainsPoint(const Point& point) const override;
  bool CrossSegment(const Segment& segm) const override;
  IShape* Clone() const override;

 private:
  int64_t x_;
  int64_t y_;
};

class Segment : public IShape {
 public:
  Segment();
  ~Segment() override = default;
  Segment(const Point& point_a, const Point& point_b);
  Segment(const Segment& segm);
  void Move(const Vector& vec) override;
  bool ContainsPoint(const Point& point) const override;
  bool CrossSegment(const Segment& segm) const override;
  IShape* Clone() const override;
  Point GetA() const;
  Point GetB() const;

 private:
  Point A_;
  Point B_;
};

class Line : public IShape {
 public:
  Line();
  ~Line() override = default;
  Line(const Point& point_a, const Point& point_b);
  Line(const Line& line);
  void Move(const Vector& vec) override;
  bool ContainsPoint(const Point& point) const override;
  bool CrossSegment(const Segment& segm) const override;
  IShape* Clone() const override;
  int64_t GetA() const;
  int64_t GetB() const;
  int64_t GetC() const;

 private:
  int64_t A_;
  int64_t B_;
  int64_t C_;
};

class Ray : public IShape {
 public:
  Ray();
  ~Ray() override = default;
  Ray(const Point& point_a, const Point& point_b);
  Ray(const Ray& ray);
  void Move(const Vector& vec) override;
  bool ContainsPoint(const Point& point) const override;
  bool CrossSegment(const Segment& segm) const override;
  IShape* Clone() const override;
  Point GetA() const;
  Vector GetVector() const;

 private:
  Vector direct_;
  Point A_;
};

class Circle : public IShape {
 public:
  Circle();
  ~Circle() override = default;
  Circle(const Point& center, int64_t radius);
  Circle(const Circle& circle);
  void Move(const Vector& vec) override;
  bool ContainsPoint(const Point& point) const override;
  bool CrossSegment(const Segment& segm) const override;
  IShape* Clone() const override;
  Point GetCentre() const;
  int64_t GetRadius() const;

 private:
  Point center_;
  int64_t radius_;
};
