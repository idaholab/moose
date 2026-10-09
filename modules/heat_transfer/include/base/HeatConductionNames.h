//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <string>
#include <optional>
#include "Units.h"

namespace HeatConduction
{
static const std::string emissivity = "emissivity";
static const std::string T_ambient = "T_ambient";

namespace Constants
{
static const Real sigma = 5.670374419e-8;

static constexpr Real
planckConstant(const std::optional<std::string> & unit)
{
  constexpr Real v = 6.62607015e-34;

  if (!unit)
    return v; // Js

  return MooseUnits(unit.value()).convert(v, MooseUnits("J*s"));
}

static constexpr Real
speedOfLight(const std::optional<std::string> & unit)
{
  constexpr Real v = 2.99792458e8;

  if (!unit)
    return v; // m/s

  return MooseUnits(unit.value()).convert(v, MooseUnits("m/s"));
}

static constexpr Real
boltzmannConstant(const std::optional<std::string> & unit)
{
  constexpr Real v = 1.380649e-23;

  if (!unit)
    return v; // J/K

  return MooseUnits(unit.value()).convert(v, MooseUnits("J/K"));
}
}
}
