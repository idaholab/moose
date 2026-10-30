//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPrimaryTimeDerivative.h"

registerKokkosResidualObject("ChemicalReactionsApp", KokkosPrimaryTimeDerivative);

InputParameters
KokkosPrimaryTimeDerivative::validParams()
{
  InputParameters params = KokkosTimeDerivative::validParams();
  params.addClassDescription("Derivative of primary species concentration wrt time");
  return params;
}

KokkosPrimaryTimeDerivative::KokkosPrimaryTimeDerivative(const InputParameters & parameters)
  : KokkosTimeDerivative(parameters), _porosity(getKokkosMaterialProperty<Real>("porosity"))
{
}
