//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosEqualValueConstraint.h"

registerKokkosMortarConstraint("MooseApp", KokkosEqualValueConstraint);

InputParameters
KokkosEqualValueConstraint::validParams()
{
  InputParameters params = MortarConstraint::validParams();
  params.addClassDescription("Constrains the value of a variable to be the same on both sides of "
                             "an interface with a Lagrange multiplier.");
  return params;
}

KokkosEqualValueConstraint::KokkosEqualValueConstraint(const InputParameters & parameters)
  : MortarConstraint(parameters)
{
}
