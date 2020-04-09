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

/**
 * Derivative free energy material defining polynomial free energies for single component materials,
 * with derivatives from ExpressionBuilder
 */
class PolynomialFreeEnergy : public DerivativeParsedMaterialHelper, public ExpressionBuilder
{
public:
  static InputParameters validParams();

  PolynomialFreeEnergy(const InputParameters & parameters);

protected:
  ///Concentration variable used in the free energy expression
  EBTerm _c;

  ///Equilibrium concentration
  EBTerm _a;

  ///Barrier height
  EBTerm _W;

  ///Polynomial order
  MooseEnum _order;
};
