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
 *  ElectromagneticConstants contains various constants useful in the
 *  Electromagnetic module, such as the complex number j, vacuum electric
 *  permittivity, etc.
 */
namespace EM
{
/// Complex number "j" (also known as "i")
static const std::complex<double> j(0, 1);

/// Magnetic permeability of free space in SI units (H/m)
static const Real mu_0 = 4.0 * libMesh::pi * 1.0e-7;

/// Speed of light in vacuum in SI units (m/s)
static const Real c = 299792458;

/// Electric permittivity of free space in SI units (F/m)
static const Real eps_0 = 1 / (mu_0 * c * c);
} // namespace EM
