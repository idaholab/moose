//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRealAux.h"

template <>
InputParameters
validParams<MaterialRealAux>()
{
  InputParameters params = validParams<MaterialAuxBase<Real>>();
  params.addClassDescription("Outputs element volume-averaged material properties");
  return params;
}

MaterialRealAux::MaterialRealAux(const InputParameters & parameters)
  : MaterialAuxBase<Real>(parameters)
{
}

Real
MaterialRealAux::getRealValue()
{
  return _prop[_qp];
}
