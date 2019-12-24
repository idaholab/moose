//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DerivativeParsedMaterialHelper.h"
#include "ExpressionBuilder.h"

// Forward Declarations
class MatrixEBTest;

template <>
InputParameters validParams<MatrixEBTest>();

/**
 * Grain boundary energy parameters for isotropic uniform grain boundary energies
 */
class MatrixEBTest : public DerivativeParsedMaterialHelper, public ExpressionBuilder
{
public:
  MatrixEBTest(const InputParameters & parameters);
};
