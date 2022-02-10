//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

namespace THM3Eqn
{
/// Number of solution variables, plus cross-sectional area
static const unsigned int N_CONS_VAR = 4;
/// Indices for solution variables, plus cross-sectional area
enum VariableIndex
{
  CONS_VAR_RHOA = 0,
  CONS_VAR_RHOUA = 1,
  CONS_VAR_RHOEA = 2,
  CONS_VAR_AREA = 3
};

/// Number of equations
static const unsigned int N_EQ = 3;
/// Indices for equations
enum EquationIndex
{
  EQ_MASS = 0,
  EQ_MOMENTUM = 1,
  EQ_ENERGY = 2
};
}
