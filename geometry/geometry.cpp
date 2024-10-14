#include "geometry.hpp"

Vector::Vector() : x_(0), y_(0) {}
Vector::Vector(int64_t coord_x, int64_t coord_y) : x_(coord_x), y_(coord_y) {}

int64_t Vector::operator*(const Vector& vec) const {
  return x_ * vec.x_ + y_ * vec.y_;
}
int64_t Vector::operator^(const Vector& vec) const {
  return x_ * vec.y_ - vec.x_ * y_;
}
Vector& Vector::operator+=(const Vector& vec) {
  x_ += vec.x_;
  y_ += vec.y_;
  return *this;
}
Vector Vector::operator+(const Vector& vec) const {
  Vector copy = *this;
  return copy += vec;
}
Vector& Vector::operator-=(const Vector& vec) {
  x_ -= vec.x_;
  y_ -= vec.y_;
  return *this;
}
Vector Vector::operator-(const Vector& vec) const {
  Vector copy = *this;
  return copy -= vec;
}
Vector& Vector::operator*=(int64_t num) {
  x_ *= num;
  y_ *= num;
  return *this;
}
Vector& Vector::operator-() { return *this *= (-1); }
int64_t Vector::GetX() const { return x_; }
int64_t Vector::GetY() const { return y_; }
int64_t Vector::ModulSquare() const { return x_ * x_ + y_ * y_; }
bool Vector::operator==(const Vector& vec) const {
  return x_ == vec.x_ && y_ == vec.y_;
}

Vector operator*(int64_t num, const Vector& vec) {
  Vector copy = vec;
  return copy *= num;
}

Vector operator*(const Vector& vec, int64_t num) {
  Vector copy = vec;
  return copy *= num;
}
bool Vector::CollinearWith(const Vector& vec) const {
  return x_ * vec.y_ == y_ * vec.x_;
}
bool Vector::CollinearSameDirect(const Vector& vec) const {
  return x_ * vec.y_ == y_ * vec.x_ && x_ * vec.x_ >= 0 && y_ * vec.y_ >= 0;
}

Point::Point() : x_(0), y_(0) {}
Point::Point(int64_t coord_x, int64_t coord_y) : x_(coord_x), y_(coord_y) {}
Point::Point(const Point& point) : x_(point.x_), y_(point.y_) {}
int64_t Point::GetX() const { return x_; }
int64_t Point::GetY() const { return y_; }
void Point::Move(const Vector& vec) {
  x_ += vec.GetX();
  y_ += vec.GetY();
}
bool Point::operator==(const Point& point) const {
  return x_ == point.x_ && y_ == point.y_;
}
Vector Point::operator-(const Point& point) const {
  return Vector(x_ - point.x_, y_ - point.y_);
}
bool Point::ContainsPoint(const Point& point) const { return point == *this; }
bool Point::CrossSegment(const Segment& segm) const {
  return segm.ContainsPoint(*this);
}
IShape* Point::Clone() const { return dynamic_cast<IShape*>(new Point(*this)); }

Segment::Segment() : A_(0, 0), B_(0, 0) {}
Segment::Segment(const Point& point_a, const Point& point_b)
    : A_(point_a), B_(point_b) {}
Segment::Segment(const Segment& segm) : A_(segm.A_), B_(segm.B_) {}
void Segment::Move(const Vector& vec) {
  A_.Move(vec);
  B_.Move(vec);
}
bool Segment::ContainsPoint(const Point& point) const {
  return ((point - A_).CollinearSameDirect(B_ - A_) &&
          (point - A_).ModulSquare() <= (B_ - A_).ModulSquare()) ||
         (point == A_ || point == B_);
}
bool Segment::CrossSegment(const Segment& segm) const {
  if (ContainsPoint(segm.A_) || ContainsPoint(segm.B_) ||
      segm.ContainsPoint(A_) || segm.ContainsPoint(B_)) {
    return true;
  }
  if ((A_ - B_).CollinearSameDirect(A_ - segm.B_) ||
      (A_ - B_).CollinearSameDirect(segm.B_ - segm.A_)) {
    return false;
  }
  return ((A_ - segm.GetA()) ^ (A_ - segm.GetB())) *
                 ((B_ - segm.GetB()) ^ (B_ - segm.GetA())) >=
             0 &&
         ((segm.GetA() - A_) ^ (segm.GetA() - B_)) *
                 ((segm.GetB() - B_) ^ (segm.GetB() - A_)) >=
             0;
}
Point Segment::GetA() const { return A_; }
Point Segment::GetB() const { return B_; }
IShape* Segment::Clone() const {
  return dynamic_cast<IShape*>(new Segment(*this));
}

Line::Line() : A_(0), B_(0), C_(0) {}
Line::Line(const Point& point_a, const Point& point_b) {
  Vector direct = point_a - point_b;
  A_ = direct.GetY();
  B_ = -direct.GetX();
  C_ = -(A_ * point_a.GetX() + B_ * point_a.GetY());
}
Line::Line(const Line& line) : A_(line.A_), B_(line.B_), C_(line.C_) {}
void Line::Move(const Vector& vec) {
  C_ -= (A_ * vec.GetX() + B_ * vec.GetY());
}
bool Line::ContainsPoint(const Point& point) const {
  return A_ * point.GetX() + B_ * point.GetY() + C_ == 0;
}
bool Line::CrossSegment(const Segment& segm) const {
  return (A_ * segm.GetA().GetX() + B_ * segm.GetA().GetY() + C_) *
             (A_ * segm.GetB().GetX() + B_ * segm.GetB().GetY() + C_) <=
         0;
}
int64_t Line::GetA() const { return A_; }
int64_t Line::GetB() const { return B_; }
int64_t Line::GetC() const { return C_; }
IShape* Line::Clone() const { return dynamic_cast<IShape*>(new Line(*this)); }

Ray::Ray() : A_(0, 0) {}
Ray::Ray(const Point& point_a, const Point& point_b)
    : direct_(point_b - point_a), A_(point_a) {}
Ray::Ray(const Ray& ray) : direct_(ray.direct_), A_(ray.A_) {}
void Ray::Move(const Vector& vec) { A_.Move(vec); }
bool Ray::ContainsPoint(const Point& point) const {
  return direct_.CollinearSameDirect(point - A_);
}
bool Ray::CrossSegment(const Segment& segm) const {
  Point point(A_);
  point.Move(direct_);
  Line ray_line(A_, point);
  if (direct_.CollinearWith(segm.GetA() - A_) &&
      !direct_.CollinearSameDirect(segm.GetA() - A_)) {
    return ContainsPoint(segm.GetB());
  }
  if (direct_.CollinearWith(segm.GetB() - A_) &&
      !direct_.CollinearSameDirect(segm.GetB() - A_)) {
    return ContainsPoint(segm.GetA());
  }
  Line line(A_, segm.GetA());
  return (line.GetA() * segm.GetB().GetX() + line.GetB() * segm.GetB().GetY() +
          line.GetC()) *
                 (line.GetA() * point.GetX() + line.GetB() * point.GetY() +
                  line.GetC()) >=
             0 &&
         ray_line.CrossSegment(segm);
}
Point Ray::GetA() const { return A_; }
Vector Ray::GetVector() const { return direct_; }
IShape* Ray::Clone() const { return dynamic_cast<IShape*>(new Ray(*this)); }

Circle::Circle() : radius_(0) {}
Circle::Circle(const Point& center, int64_t radius)
    : center_(center), radius_(radius) {}
Circle::Circle(const Circle& circle)
    : center_(circle.center_), radius_(circle.radius_) {}
void Circle::Move(const Vector& vec) { center_.Move(vec); }
bool Circle::ContainsPoint(const Point& point) const {
  return (center_ - point).ModulSquare() <= radius_ * radius_;
}
bool Circle::CrossSegment(const Segment& segm) const {
  if (ContainsPoint(segm.GetA()) ^ ContainsPoint(segm.GetB())) {
    return true;
  }
  if ((center_ - segm.GetA()).ModulSquare() < radius_ * radius_ &&
      (center_ - segm.GetB()).ModulSquare() < radius_ * radius_) {
    return false;
  }
  return ((center_ - segm.GetA()) ^ (center_ - segm.GetB())) *
                 ((center_ - segm.GetA()) ^ (center_ - segm.GetB())) <=
             radius_ * radius_ * (segm.GetA() - segm.GetB()).ModulSquare() &&
         (((segm.GetA() - center_) * (segm.GetA() - segm.GetB())) >= 0 &&
          ((segm.GetB() - center_) * (segm.GetB() - segm.GetA())) >= 0);
}
Point Circle::GetCentre() const { return center_; }
int64_t Circle::GetRadius() const { return radius_; }
IShape* Circle::Clone() const {
  return dynamic_cast<IShape*>(new Circle(*this));
}
