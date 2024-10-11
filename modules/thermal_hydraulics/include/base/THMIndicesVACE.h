//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

namespace THMVACE3D
{
/// Number of numerical flux function inputs for 3D
static const unsigned int N_FLUX_INPUTS = 6;
/// Indices of numerical flux function inputs for 3D
enum FluxIn
{
  RHOA = 0,
  RHOUA = 1,
  RHOVA = 2,
  RHOWA = 3,
  RHOEA = 4,
  AREA = 5
};

/// Number of numerical flux function outputs for 3D
static const unsigned int N_FLUX_OUTPUTS = 5;
/// Indices of numerical flux function outputs for 3D
enum FluxOut
{
  MASS = 0,
  MOM_NORM = 1,
  MOM_TAN1 = 2,
  MOM_TAN2 = 3,
  ENERGY = 4
};
}

namespace THMVACE1D
{
/// Number of numerical flux function inputs for 1D
static const unsigned int N_FLUX_INPUTS = 4;
/// Indices of numerical flux function inputs for 1D
enum FluxIn
{
  RHOA = 0,
  RHOUA = 1,
  RHOEA = 2,
  AREA = 3
};

/// Number of numerical flux function outputs for 1D
static const unsigned int N_FLUX_OUTPUTS = 3;
/// Indices of numerical flux function outputs for 1D
enum FluxOut
{
  MASS = 0,
  MOMENTUM = 1,
  ENERGY = 2
};

// Number of primitive variables for 1D
static const unsigned int N_PRIM_VARS = 3;
/// Indices for primitive variables for 1D
enum PrimVar
{
  PRESSURE = 0,
  VELOCITY = 1,
  TEMPERATURE = 2
};
}
