//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

/**
 *  ElectromagneticEnums contains various enumerations useful in the
 *  Electromagnetic module, such as real/imag component definitions in
 *  Kernels, BCs, etc.
 */
namespace EM
{
/// Enum used when determining the component of a field vector or complex coefficient
enum ComponentEnum
{
  REAL,
  IMAGINARY
};

/// Enum used when determining the function of a Robin-style boundary condition
enum RobinEnum
{
  ABSORBING, ///< Case when the boundary is configured to absorb impinging electromagnetic radiation

  PORT ///< Case when the boundary is configured to both absorb impinging electromagnetic radiation
       ///< and launch an incoming pre-defined electromagnetic wave from the boundary into the
       ///< simulation domain
};
} // namespace EM
