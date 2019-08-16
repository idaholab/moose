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
class EBMoelansFLoc;

template <>
InputParameters validParams<EBMoelansFLoc>();

/**
 * Grain boundary energy parameters for isotropic uniform grain boundary energies
 */
class EBMoelansFLoc : public DerivativeParsedMaterialHelper<>,
		     public ExpressionBuilder
{
public:
  EBMoelansFLoc(const InputParameters & parameters);

protected:
  const unsigned int _op_num;
  std::vector<EBTerm> _vals;
  const Real _gamma;
};
