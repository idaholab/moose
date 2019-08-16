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
class GBEBLocation;

template <>
InputParameters validParams<GBEBLocation>();

/**
 * Grain boundary energy parameters for isotropic uniform grain boundary energies
 */
class GBEBLocation : public DerivativeParsedMaterialHelper<RealVectorValue>,
		     public ExpressionBuilder
{
public:
  GBEBLocation(const InputParameters & parameters);

protected:
  const unsigned int _op_num;
  std::vector<EBTerm> _vals;
  const int _thickness;
  const int _sharpness;
};
