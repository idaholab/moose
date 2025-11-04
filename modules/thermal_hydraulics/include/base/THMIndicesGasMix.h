//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

namespace THMGasMix3D
{
/// Number of numerical flux function inputs for 3D
static const unsigned int N_FLUX_INPUTS = 7;
/// Indices of numerical flux function inputs for 3D
enum FluxIn
{
  XIRHOA = 0,
  RHOA = 1,
  RHOUA = 2,
  RHOVA = 3,
  RHOWA = 4,
  RHOEA = 5,
  AREA = 6
};

/// Number of numerical flux function outputs for 3D
static const unsigned int N_FLUX_OUTPUTS = 6;
/// Indices of numerical flux function outputs for 3D
enum FluxOut
{
  SPECIES = 0,
  MASS = 1,
  MOM_NORM = 2,
  MOM_TAN1 = 3,
  MOM_TAN2 = 4,
  ENERGY = 5
};
}

namespace THMGasMix1D
{
/// Number of numerical flux function inputs
static const unsigned int N_FLUX_INPUTS = 5;
/// Indices of numerical flux function inputs
enum FluxIn
{
  XIRHOA = 0,
  RHOA = 1,
  RHOUA = 2,
  RHOEA = 3,
  AREA = 4
};

/// Number of numerical flux function outputs
static const unsigned int N_FLUX_OUTPUTS = 4;
/// Indices of numerical flux function outputs
enum FluxOut
{
  SPECIES = 0,
  MASS = 1,
  MOMENTUM = 2,
  ENERGY = 3
};

// Number of primitive variables
static const unsigned int N_PRIM_VARS = 4;
/// Indices for primitive variables
enum PrimVar
{
  MASS_FRACTION = 0,
  PRESSURE = 1,
  VELOCITY = 2,
  TEMPERATURE = 3
};
}
