//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseError.h"
#include <vector>
#include <iostream>

class MooseUnits;

// power function injected into std
namespace std
{
MooseUnits pow(const MooseUnits &, int);
}

// output stream operator
std::ostream & operator<<(std::ostream & os, const MooseUnits & u);

/**
 * Physical unit management class with runtime unit string parsing, unit checking,
 * unit conversion, and output.
 */
class MooseUnits
{
  enum class BaseUnit
  {
    METER,
    KILOGRAM,
    SECOND,
    AMPERE,
    KELVIN,
    COUNT,
    CANDELA
  };

public:
  MooseUnits(const std::string & unit_string);
  MooseUnits(Real f) : _factor(f), _shift(), _base() {}
  MooseUnits(Real f, Real s, std::vector<std::pair<MooseUnits::BaseUnit, int>> b)
    : _factor(f), _shift(s), _base(b)
  {
  }

  /// checks if the units are dimensionally conforming (i.e. the describe the same physical quanitity)
  bool conformsTo(const MooseUnits &) const;

  /// Converts `from_value` in units of `from_units` to value this units
  Real convert(Real from_value, const MooseUnits & from_unit) const;

  /// parse a unit string into a MooseUnits object
  void parse(const std::string & unit_string);

  /// simplify into the canonical form that permits comparisons
  void simplify();

  ///@{ data tables with SI prefixes and known units
  static const std::map<std::string, Real> _si_prefix;
  static const std::vector<std::pair<std::string, MooseUnits>> _unit_table;
  ///@}

  ///@{ query the nature of the unit
  bool isLength() const { return isBase(BaseUnit::METER); }
  bool isTime() const { return isBase(BaseUnit::SECOND); }
  bool isMass() const { return isBase(BaseUnit::KILOGRAM); }
  bool isCurrent() const { return isBase(BaseUnit::AMPERE); }
  bool isTemperature() const { return isBase(BaseUnit::KELVIN); }
  ///@}

  /// Unit prefactor scaling
  MooseUnits operator*(const Real f) const;

  /**
   * @{ Unit combination operators for unit pairs zero out the shift. e.g. temperatures
   * are interpreted as temperature differentials
   */
  MooseUnits operator*(const MooseUnits & rhs) const;
  MooseUnits operator/(const MooseUnits & rhs) const;
  ///@}

  bool operator==(const MooseUnits & rhs) const;
  bool operator==(const Real rhs) const;

  /// cast of units representing pure numbers
  explicit operator Real() const;

  friend std::ostream & operator<<(std::ostream & os, const MooseUnits & dt);
  friend MooseUnits std::pow(const MooseUnits &, int);

  ///@{ iostream manipulators
  static std::ostream & latex(std::ostream & os);
  static std::ostream & text(std::ostream & os);
  ///@}

protected:
  /// helper function to generate a pretty mooseError
  template <typename... Args>
  void parseError(const std::string & unit_string, std::string::const_iterator it, Args... args);

  /// conversion factor w.r.t. the base SI units
  Real _factor;

  /// additive shift (for Celsius and Fahrenheit)
  Real _shift;

  /// check if the unit has a pure base
  bool isBase(const MooseUnits::BaseUnit) const;

  /// base SI units and their exponents
  std::vector<std::pair<BaseUnit, int>> _base;

  /// iosteam manipulator helper to toggle latex / text output
  static int geti();
};

template <typename... Args>
void
MooseUnits::parseError(const std::string & unit_string,
                       std::string::const_iterator it,
                       Args... args)
{
  auto d = std::distance(unit_string.begin(), it);
  mooseError("At position ", d, " in ", unit_string, ": ", std::forward<Args>(args)...);
}
