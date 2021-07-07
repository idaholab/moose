//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Units.h"

template <bool>
class GenericRealU;

// Port to std
namespace std
{
// Power
template <bool is_ad>
GenericRealU<is_ad>
pow(const GenericRealU<is_ad> & a, int b)
{
  return GenericRealU<is_ad>(pow(a.value(), b), pow(a.unit(), b));
}

// Absolute value
template <bool is_ad>
GenericRealU<is_ad>
abs(const GenericRealU<is_ad> & a)
{
  return GenericRealU<is_ad>(abs(a.value()), a.unit());
}
}

// The GenericReal with a const unit.
template <bool is_ad>
class GenericRealU
{
public:
  /// Empty constructor
  GenericRealU() : _value(0), _unit("1") {}

  /// Construct from a value, set the unit to be dimensionless
  GenericRealU(const GenericReal<is_ad> & value) : _value(value), _unit("1") {}

  // @{ Construct from a unit, set the value to 0
  GenericRealU(const MooseUnits & unit) : _value(0), _unit(unit) {}
  GenericRealU(const std::string & unit) : _value(0), _unit(unit) {}
  // @}

  // @{ Construct from a value and a unit
  GenericRealU(const GenericReal<is_ad> & value, const MooseUnits & unit)
    : _value(value), _unit(unit)
  {
  }
  GenericRealU(const GenericReal<is_ad> & value, const std::string & unit)
    : _value(value), _unit(unit)
  {
  }
  // @}

  // @{ Assignment operators
  /// Assignment from a value, assuming same units
  GenericRealU & operator=(const GenericReal<is_ad> & value)
  {
    _value = value;
    return *this;
  }
  /// Assignment from another GenericRealU
  GenericRealU & operator=(const GenericRealU & rhs)
  {
    _value = _unit.convert(rhs._value, rhs._unit);
    return *this;
  }
  // @}

  // @{ Addition
  GenericRealU operator+(const GenericReal<is_ad> & value) const
  {
    return GenericRealU(_value + value, _unit);
  }
  GenericRealU operator+(const GenericRealU & rhs) const
  {
    return GenericRealU(_value + _unit.convert(rhs._value, rhs._unit), _unit);
  }
  void operator+=(const GenericReal<is_ad> & value) { _value += value; }
  void operator+=(const GenericRealU & rhs) { _value += _unit.convert(rhs._value, rhs._unit); }
  // @}

  // @{ Subtraction
  GenericRealU operator-(const GenericReal<is_ad> & value) const
  {
    return GenericRealU(_value - value, _unit);
  }
  GenericRealU operator-(const GenericRealU & rhs) const
  {
    return GenericRealU(_value - _unit.convert(rhs._value, rhs._unit), _unit);
  }
  void operator-=(const GenericReal<is_ad> & value) { _value -= value; }
  void operator-=(const GenericRealU & rhs) { _value -= _unit.convert(rhs._value, rhs._unit); }
  // @}

  // @{ Multiplication
  GenericRealU operator*(const GenericReal<is_ad> & value) const
  {
    return GenericRealU(_value * value, _unit * _unit);
  }
  GenericRealU operator*(const GenericRealU & rhs) const
  {
    return GenericRealU(_value * _unit.convert(rhs._value, rhs._unit), _unit * _unit);
  }
  // @}

  // @{ Division
  GenericRealU operator/(const GenericReal<is_ad> & value) const
  {
    return GenericRealU(_value / value, _unit / _unit);
  }
  GenericRealU operator/(const GenericRealU & rhs) const
  {
    return GenericRealU(_value / _unit.convert(rhs._value, rhs._unit), _unit / _unit);
  }
  // @}

  // @{ Comparison
  bool operator==(const GenericRealU & rhs) const
  {
    return _value == _unit.convert(rhs._value, rhs._unit);
  }
  bool operator>(const GenericRealU & rhs) const
  {
    return _value > _unit.convert(rhs._value, rhs._unit);
  }
  bool operator<(const GenericRealU & rhs) const
  {
    return _value < _unit.convert(rhs._value, rhs._unit);
  }
  bool operator>=(const GenericRealU & rhs) const
  {
    return _value >= _unit.convert(rhs._value, rhs._unit);
  }
  bool operator<=(const GenericRealU & rhs) const
  {
    return _value <= _unit.convert(rhs._value, rhs._unit);
  }
  // @}

  const GenericReal<is_ad> & value() const { return _value; }
  const MooseUnits & unit() const { return _unit; }

protected:
private:
  GenericReal<is_ad> _value;
  const MooseUnits _unit;
};

typedef GenericRealU<false> RealU;
typedef GenericRealU<true> ADRealU;
