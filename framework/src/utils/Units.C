//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Units.h"
#include "libmesh/int_range.h"

#include <stack>
#include <algorithm>
#include <sstream>

const std::map<std::string, Real> MooseUnits::_si_prefix = {
    {"Y", 1e24}, {"Z", 1e21},  {"E", 1e18},  {"P", 1e15},  {"T", 1e12},  {"G", 1e9},  {"M", 1e6},
    {"k", 1e3},  {"h", 1e2},   {"da", 1e1},  {"d", 1e-1},  {"c", 1e-2},  {"m", 1e-3}, {"mu", 1e-6},
    {"n", 1e-9}, {"p", 1e-12}, {"f", 1e-15}, {"a", 1e-18}, {"z", 1e-21}, {"y", 1e-24}};

const std::vector<std::pair<std::string, MooseUnits>> MooseUnits::_unit_table = {
    // avoid matching meters
    {"Ohm",
     {
         1, // conversion factor
         0, // additive shift (currently only used for Celsius and Fahrenheit)
         {{MooseUnits::BaseUnit::KILOGRAM, 1},
          {MooseUnits::BaseUnit::METER, 2},
          {MooseUnits::BaseUnit::SECOND, -3},
          {MooseUnits::BaseUnit::AMPERE, -2}},
     }},
    {"atm",
     {101.325e3,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, -1},
       {MooseUnits::BaseUnit::SECOND, -2}}}}, // Standard atmosphere
    // avoid matching volts
    {"eV",
     {
         1.602176634e-19,
         0,
         {{MooseUnits::BaseUnit::KILOGRAM, 1},
          {MooseUnits::BaseUnit::METER, 2},
          {MooseUnits::BaseUnit::SECOND, -2}},
     }}, // electronvolt
    // avoid matching grams
    {"erg",
     {
         1e-7,
         0,
         {{MooseUnits::BaseUnit::KILOGRAM, 1},
          {MooseUnits::BaseUnit::METER, 2},
          {MooseUnits::BaseUnit::SECOND, -2}},
     }}, // erg

    // avoid matching Farad or Coulomb
    {"degC", {1, 273.15, {{MooseUnits::BaseUnit::KELVIN, 1}}}},         // Celsius
    {"degF", {5.0 / 9.0, 459.67, {{MooseUnits::BaseUnit::KELVIN, 1}}}}, // Fahrenheit
    {"degR", {5.0 / 9.0, 0, {{MooseUnits::BaseUnit::KELVIN, 1}}}},      // Rankine

    {"Ang", {1e-10, 0, {{MooseUnits::BaseUnit::METER, 1}}}}, // Angstrom

    // Base units
    {"m", {1, 0, {{MooseUnits::BaseUnit::METER, 1}}}},
    {"g", {0.001, 0, {{MooseUnits::BaseUnit::KILOGRAM, 1}}}},
    {"s", {1, 0, {{MooseUnits::BaseUnit::SECOND, 1}}}},
    {"A", {1, 0, {{MooseUnits::BaseUnit::AMPERE, 1}}}},
    {"K", {1, 0, {{MooseUnits::BaseUnit::KELVIN, 1}}}},
    {"mol", {6.02214076e23, 0, {{MooseUnits::BaseUnit::COUNT, 1}}}},
    {"cd", {1, 0, {{MooseUnits::BaseUnit::CANDELA, 1}}}},

    // Derived units
    {"N",
     {1,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, 1},
       {MooseUnits::BaseUnit::SECOND, -2}}}}, // Newton
    {"Pa",
     {1,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, -1},
       {MooseUnits::BaseUnit::SECOND, -2}}}}, // Pascal
    {"J",
     {1,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, 2},
       {MooseUnits::BaseUnit::SECOND, -2}}}}, // Joule
    {"W",
     {1,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, 2},
       {MooseUnits::BaseUnit::SECOND, -3}}}}, // Watt
    {"C",
     {1, 0, {{MooseUnits::BaseUnit::AMPERE, 1}, {MooseUnits::BaseUnit::SECOND, 1}}}}, // Coulomb
    {"V",
     {1,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, 2},
       {MooseUnits::BaseUnit::SECOND, -3},
       {MooseUnits::BaseUnit::AMPERE, -1}}}},
    {"F",
     {1,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, -1},
       {MooseUnits::BaseUnit::METER, -2},
       {MooseUnits::BaseUnit::SECOND, 4},
       {MooseUnits::BaseUnit::AMPERE, 2}}}},
    {"S",
     {1,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, -1},
       {MooseUnits::BaseUnit::METER, -2},
       {MooseUnits::BaseUnit::SECOND, 3},
       {MooseUnits::BaseUnit::AMPERE, 2}}}}, // Siemens = 1/Ohm (electrical conductance)
    {"Wb",
     {1,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, 2},
       {MooseUnits::BaseUnit::SECOND, -2},
       {MooseUnits::BaseUnit::AMPERE, -1}}}}, // Weber (magnetic flux)
    {"T",
     {1,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::SECOND, -2},
       {MooseUnits::BaseUnit::AMPERE, -1}}}}, // Tesla (magnetic flux density)
    {"H",
     {1,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, 2},
       {MooseUnits::BaseUnit::SECOND, -2},
       {MooseUnits::BaseUnit::AMPERE, -2}}}}, // Henry (inductance)

    // cgs units
    {"Ba",
     {0.1,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, -1},
       {MooseUnits::BaseUnit::SECOND,
        -2}}}}, // barye (unit of pressure - not to be confused with bar)
    {"dyn",
     {1e-5,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, 1},
       {MooseUnits::BaseUnit::SECOND, -2}}}}, // dyne

    // Freedom units
    {"ft", {0.3048, 0, {{MooseUnits::BaseUnit::METER, 1}}}},        // foot
    {"in", {25.4e-3, 0, {{MooseUnits::BaseUnit::METER, 1}}}},       // inch
    {"lb", {0.45359237, 0, {{MooseUnits::BaseUnit::KILOGRAM, 1}}}}, // pound of mass
    {"lbf",
     {4.4482216152605,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, 1},
       {MooseUnits::BaseUnit::SECOND, -2}}}}, // pound of force
    {"psi",
     {6.894757e3,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, -1},
       {MooseUnits::BaseUnit::SECOND, -2}}}}, // pound-force per square inch

    // misc.
    {"BTU",
     {1055.06,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, 2},
       {MooseUnits::BaseUnit::SECOND, -2}}}}, // ISO 31-4 British thermal unit
    {"bar",
     {1e5,
      0,
      {{MooseUnits::BaseUnit::KILOGRAM, 1},
       {MooseUnits::BaseUnit::METER, -1},
       {MooseUnits::BaseUnit::SECOND, -2}}}},                        // bar (unit of pressure)
    {"h", {60 * 60, 0, {{MooseUnits::BaseUnit::SECOND, 1}}}},        // hour
    {"day", {60 * 60 * 24, 0, {{MooseUnits::BaseUnit::SECOND, 1}}}}, // day
    {"l", {1e-3, 0, {{MooseUnits::BaseUnit::METER, 3}}}},            // liter
    {"u",
     {1.66053906660e-27, 0, {{MooseUnits::BaseUnit::KILOGRAM, 3}}}}, // unified atomic mass unit
    {"at", {1, 0, {{MooseUnits::BaseUnit::COUNT, 1}}}}               // 1 single count (atom)
};

MooseUnits::MooseUnits(const std::string & unit_string) : _factor(1.0), _shift(0.0), _base()
{
  // parse the passed in unit string
  parse(unit_string);
}

bool
MooseUnits::conformsTo(const MooseUnits & u) const
{
  if (_base.size() != u._base.size())
    return false;
  for (std::size_t i = 0; i < _base.size(); ++i)
    if (_base[i] != u._base[i])
      return false;

  return true;
}

Real
MooseUnits::convert(Real from_value, const MooseUnits & from_unit) const
{
  if (!conformsTo(from_unit))
    mooseError("Cannot convert between non-conforming units '", *this, "' and '", from_unit, "'.");
  return ((from_value + from_unit._shift) * from_unit._factor) / _factor - _shift;
}

void
MooseUnits::parse(const std::string & unit_string)
{
  // we need to understand * / ^int ( ) string
  // m^2*kg/(N*s)^2
  // no numerical expressions are permitted (e.g. no 1/2 0.5 4*2)

  // parse stack
  std::stack<std::vector<std::pair<MooseUnits::BaseUnit, int>>> stack;
  std::stack<char> op_stack;
  std::stack<MooseUnits> out;

  auto it = unit_string.begin();
  const auto end = unit_string.end();

  do
  {
    // skip whitespace
    if (*it == ' ')
    {
      it++;
      continue;
    }
    // opening parenthesis
    else if (*it == '(')
      op_stack.push(*(it++));
    // multiply or divide
    else if (*it == '*' || *it == '/')
    {
      // pop */ off
      while (!op_stack.empty() && (op_stack.top() == '*' || op_stack.top() == '/'))
      {
        auto top = op_stack.top();
        op_stack.pop();

        if (out.size() < 2)
          parseError(
              unit_string, it, "Applying ", top, " but don't have enough items on the stack.");
        auto rhs = out.top();
        out.pop();
        if (top == '*')
          out.top() = out.top() * rhs;
        else
          out.top() = out.top() / rhs;
      }

      op_stack.push(*(it++));
    }
    // closing parenthesis
    else if (*it == ')')
    {
      do
      {
        if (op_stack.empty())
          parseError(unit_string, it, "Mismatching right parenthesis.");

        auto top = op_stack.top();
        op_stack.pop();

        if (top == '(')
          break;

        if (top == '*' || top == '/')
        {
          if (out.size() < 2)
            parseError(
                unit_string, it, "Applying ", top, " but don't have enough items on the stack.");
          auto rhs = out.top();
          out.pop();
          if (top == '*')
            out.top() = out.top() * rhs;
          else
            out.top() = out.top() / rhs;
        }
      } while (true);
      it++;
    }
    // exponent
    else if (*it == '^')
    {
      // check for sign
      int sign = 1;
      ++it;
      if (it != end && *it == '-')
      {
        sign = -1;
        it++;
      }

      int num = 0;
      bool valid = false;
      while (*it >= '0' && *it <= '9')
      {
        num *= 10;
        num += *(it++) - '0';
        valid = true;
      }

      // error
      if (!valid)
      {
        if (it == end)
          parseError(unit_string, it, "Expected a number but found end of string.");
        else
          parseError(unit_string, it, "Expected a number but got '", (*it), "'.");
      }

      // exponents do not get pushed on the stack, but are applied immediately
      if (out.empty())
        parseError(unit_string, it, "Applying an exponent, but found no units in front of it.");
      out.top() = std::pow(out.top(), sign * num);
    }
    else
    {
      std::string unit;
      while (it != end && ((*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z') ||
                           (*it >= '0' && *it <= '9') || *it == '.' || *it == '-'))
        unit += *(it++);

      // check if a number prefix is present
      Real si_factor;
      std::stringstream ss(unit);
      if ((ss >> si_factor).fail() || !std::isfinite(si_factor))
        si_factor = 1.0;
      else
      {
        if (ss.eof())
        {
          out.push(MooseUnits(si_factor));
          continue;
        }
        ss >> unit;
      }

      // parse the unit and a potential SI prefix
      auto len = unit.length();
      if (len == 0)
      {
        if (it == end)
          parseError(unit_string, it, "Expected unit but found end of string.");
        else
          parseError(unit_string, it, "Expected unit but found '", *it, "'.");
      }

      unsigned int i = 0;
      for (; i < _unit_table.size(); ++i)
      {
        const auto & s = _unit_table[i].first;
        const auto slen = s.length();
        if (slen <= len && unit.substr(len - slen) == s)
          break;
      }

      // invalid unit
      if (i == _unit_table.size())
        parseError(unit_string, it, "Unknown unit '", unit, "'.");

      // get prefix
      const auto & s = _unit_table[i].first;
      const auto slen = s.length();
      if (slen < len)
      {
        const auto prefix = unit.substr(0, len - slen);
        auto jt = _si_prefix.find(prefix);
        if (jt != _si_prefix.end())
          si_factor = jt->second;
        else
          parseError(unit_string, it, "Unknown SI prefix '", prefix, "' on unit '", unit, "'.");
      }

      // construct a unit object and push to the output stack
      MooseUnits d = _unit_table[i].second * si_factor;
      out.push(d);
    }
  } while (it != end);

  // unwind operator stack
  while (!op_stack.empty())
  {
    auto top = op_stack.top();
    op_stack.pop();

    if (top == '(' || top == ')')
      parseError(unit_string, it, "Unit string contains unmatched parenthesis.");

    if (top == '*' || top == '/')
    {
      if (out.size() < 2)
        parseError(unit_string,
                   it,
                   "Applying ",
                   top,
                   " but don't have enough items on the stack.",
                   out.size());
      auto rhs = out.top();
      out.pop();
      if (top == '*')
        out.top() = out.top() * rhs;
      else
        out.top() = out.top() / rhs;
    }
  }

  // consistency check
  if (out.size() != 1)
    mooseError("Internal parse error. Remaining output stach size sould be 1, but is ", out.size());

  *this = out.top();
  simplify();
}

MooseUnits
MooseUnits::operator*(const Real f) const
{
  MooseUnits u = *this;
  u._factor *= f;
  u._shift *= f;
  return u;
}

MooseUnits
MooseUnits::operator*(const MooseUnits & rhs) const
{
  MooseUnits u = rhs;
  u._factor *= _factor;
  u._base.insert(u._base.end(), _base.begin(), _base.end());
  u._shift = 0.0;
  u.simplify();
  return u;
}

MooseUnits
MooseUnits::operator/(const MooseUnits & rhs) const
{
  MooseUnits u = rhs;
  u._factor = _factor / u._factor;
  for (auto & b : u._base)
    b.second *= -1;
  u._base.insert(u._base.end(), _base.begin(), _base.end());
  u._shift = 0.0;
  u.simplify();
  return u;
}

bool
MooseUnits::operator==(const MooseUnits & rhs) const
{
  if (_factor != rhs._factor || _shift != rhs._shift)
    return false;
  return conformsTo(rhs);
}

bool
MooseUnits::operator==(const Real rhs) const
{
  return (_factor == rhs && _shift == 0.0 && _base.empty());
}

MooseUnits::operator Real() const
{
  if (!_base.empty())
    mooseError("Unit '", *this, "' is not a pure number.");
  return _factor;
}

void
MooseUnits::simplify()
{
  std::map<BaseUnit, int> base_map;
  for (auto & b : _base)
  {
    auto j = base_map.find(b.first);
    if (j == base_map.end())
      base_map.insert(b);
    else
      j->second += b.second;
  }

  // replace base vector with map contents
  _base.clear();
  for (auto & u : base_map)
    if (u.second != 0)
      _base.push_back(u);
}

bool
MooseUnits::isBase(const MooseUnits::BaseUnit base) const
{
  // must have only one base unit
  if (_base.size() != 1)
    return false;

  // its exponent must be one
  if (_base[0].second != 1)
    return false;

  // and it has to be the right base unit
  return (_base[0].first == base);
}

namespace std
{
MooseUnits
pow(const MooseUnits & u, int e)
{
  MooseUnits r = u;
  r._factor = std::pow(u._factor, e);
  r._shift = 0.0;
  for (auto & b : r._base)
    b.second *= e;
  return r;
}
}

std::ostream &
MooseUnits::latex(std::ostream & os)
{
  os.iword(geti()) = 1;
  return os;
}
std::ostream &
MooseUnits::text(std::ostream & os)
{
  os.iword(geti()) = 0;
  return os;
}

int
MooseUnits::geti()
{
  static int i = std::ios_base::xalloc();
  return i;
}

std::ostream &
operator<<(std::ostream & os, const MooseUnits & u)
{
  bool latex = (os.iword(MooseUnits::geti()) == 1);
  static const std::vector<std::string> base_unit = {"m", "kg", "s", "A", "K", "at", "cd"};

  if (u._factor != 1.0 || u._base.empty())
  {
    os << u._factor;
    if (!u._base.empty())
      os << ' ';
  }

  for (const auto i : index_range(u._base))
  {
    os << (i ? (latex ? "\\cdot " : "*") : "");

    const auto & unit = base_unit[int(u._base[i].first)];
    if (latex)
      os << "\\text{" << unit << "}";
    else
      os << unit;

    if (u._base[i].second != 1)
    {
      if (latex)
        os << "^{" << u._base[i].second << '}';
      else
        os << '^' << u._base[i].second;
    }
  }

  return os;
}
