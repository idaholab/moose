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

namespace GeochemistryConstants
{
constexpr Real MOLES_PER_KG_WATER =
    55.5; // This is only approximate, but 55.5 is chosen so the results of the geochemistry module
          // closely match Geochemists Workbench
constexpr Real LOGTEN = 2.30258509299404;
constexpr Real FARADAY = 96485.3415;    // Coulombs . mol^-1
constexpr Real GAS_CONSTANT = 8.314472; // m^2 . kg . s^-2 . K^-1 . mol^-1
constexpr Real CELSIUS_TO_KELVIN = 273.15;
constexpr Real PERMITTIVITY_FREE_SPACE = 8.8541878128E-12; // F . m^-1
constexpr Real DIELECTRIC_CONSTANT_WATER = 78.5;
constexpr Real DENSITY_WATER = 1000; // kg . m^-3.  Used in surface-potential calculation
}
