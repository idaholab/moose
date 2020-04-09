#pragma once

class Fraction
{
public:
  Fraction(int numerator, int denominator) : _numerator(numerator), _denominator(denominator) {}
  Fraction(int numerator) : _numerator(numerator), _denominator(1) {}

  Fraction operator+(int rhs)
  {
    Fraction new_frac(rhs);
    return *this + new_frac;
  };
  Fraction operator-(int rhs)
  {
    Fraction new_frac(rhs);
    return *this - new_frac;
  };
  Fraction operator*(int rhs)
  {
    Fraction new_frac(rhs);
    return *this * new_frac;
  };
  Fraction operator/(int rhs)
  {
    Fraction new_frac(rhs);
    return *this / new_frac;
  };
  double operator%(int rhs) { return fmod(_numerator / _denominator, rhs); };
  bool operator<(int rhs)
  {
    Fraction new_frac(rhs);
    return *this < new_frac;
  };
  bool operator>(int rhs)
  {
    Fraction new_frac(rhs);
    return *this > new_frac;
  };
  bool operator<=(int rhs)
  {
    Fraction new_frac(rhs);
    return *this <= new_frac;
  };
  bool operator>=(int rhs)
  {
    Fraction new_frac(rhs);
    return *this >= new_frac;
  };
  bool operator==(int rhs)
  {
    Fraction new_frac(rhs);
    return *this == new_frac;
  };
  bool operator!=(int rhs)
  {
    Fraction new_frac(rhs);
    return *this != new_frac;
  };

  double operator+(double rhs) { return _numerator / _denominator + rhs; };
  double operator-(double rhs) { return _numerator / _denominator - rhs; };
  double operator*(double rhs) { return (_numerator / _denominator) * rhs; };
  double operator/(double rhs) { return (_numerator / _denominator) / rhs; };
  double operator%(double rhs) { return fmod(_numerator / _denominator, rhs); };
  bool operator<(double rhs) { return _numerator / _denominator < rhs; };
  bool operator>(double rhs) { return _numerator / _denominator > rhs; };
  bool operator<=(double rhs) { return _numerator / _denominator <= rhs; };
  bool operator>=(double rhs) { return _numerator / _denominator >= rhs; };
  bool operator==(double rhs) { return _numerator / _denominator == rhs; };
  bool operator!=(double rhs) { return _numerator / _denominator != rhs; };

  Fraction operator+(Fraction rhs)
  {
    return Fraction(_numerator * rhs._denominator + _denominator * rhs._numerator,
                    _denominator * rhs._denominator);
  };
  Fraction operator-(Fraction rhs)
  {
    return Fraction(_numerator * rhs._denominator - _denominator * rhs._numerator,
                    _denominator * rhs._denominator);
  };
  Fraction operator*(Fraction rhs)
  {
    return Fraction(_numerator * rhs._numerator, _denominator * rhs._denominator);
  };
  Fraction operator/(Fraction rhs)
  {
    return Fraction(_numerator * rhs._denominator, _denominator * rhs._numerator);
  };
  bool operator<(Fraction rhs)
  {
    return _numerator / _denominator < rhs._numerator / rhs._denominator;
  };
  bool operator>(Fraction rhs)
  {
    return _numerator / _denominator > rhs._numerator / rhs._denominator;
  };
  bool operator<=(Fraction rhs)
  {
    return _numerator / _denominator <= rhs._numerator / rhs._denominator;
  };
  bool operator>=(Fraction rhs)
  {
    return _numerator / _denominator >= rhs._numerator / rhs._denominator;
  };
  bool operator==(Fraction rhs)
  {
    return _numerator / _denominator == rhs._numerator / rhs._denominator;
  };
  bool operator!=(Fraction rhs)
  {
    return _numerator / _denominator != rhs._numerator / rhs._denominator;
  };

  Fraction operator-() { return Fraction(-_numerator, _denominator); };
  bool operator!() { return !_numerator; };
  explicit operator bool() { return _numerator; };
  operator double() { return _numerator / _denominator; }

  friend std::ostream & operator<<(std::ostream & os, Fraction rhs);

private:
  int _numerator;
  int _denominator;
};

std::ostream &
operator<<(std::ostream & os, Fraction rhs)
{
  os << rhs._numerator << "/" << rhs._denominator;
  return os;
}
