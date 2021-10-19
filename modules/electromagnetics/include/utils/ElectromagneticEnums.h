//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/** ElectromagneticEnums contains various enumerations useful in the
 * Electromagnetic module, such as real/imag component definitions in
 * Kernels, BCs, etc.
 */

namespace electromagnetics
{
enum ComponentEnum
{
  REAL,
  IMAGINARY
};
enum RobinEnum
{
  ABSORBING,
  PORT
};
} // namespace electromagnetics
