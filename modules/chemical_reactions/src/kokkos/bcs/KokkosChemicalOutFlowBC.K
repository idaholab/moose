//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosChemicalOutFlowBC.h"

registerKokkosResidualObject("ChemicalReactionsApp", KokkosChemicalOutFlowBC);

InputParameters
KokkosChemicalOutFlowBC::validParams()
{
  InputParameters params = IntegratedBCValue::validParams();
  params.addClassDescription("Chemical flux boundary condition");
  return params;
}

KokkosChemicalOutFlowBC::KokkosChemicalOutFlowBC(const InputParameters & parameters)
  : IntegratedBCValue(parameters),
    _diff(getKokkosMaterialProperty<Real>("diffusivity")),
    _porosity(getKokkosMaterialProperty<Real>("porosity"))
{
}
