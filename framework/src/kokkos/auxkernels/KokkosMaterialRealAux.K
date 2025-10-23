//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMaterialRealAux.h"

registerKokkosAuxKernel("MooseApp", KokkosMaterialRealAux);

InputParameters
KokkosMaterialRealAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredParam<MaterialPropertyName>("property", "The material property name.");
  params.addParam<Real>(
      "factor", 1, "The factor by which to multiply your material property for visualization");
  params.addParam<Real>(
      "offset", 0, "The offset to add to your material property for visualization");
  params.addClassDescription("Outputs element volume-averaged material properties");

  return params;
}

KokkosMaterialRealAux::KokkosMaterialRealAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _prop(getKokkosMaterialProperty<Real>("property")),
    _factor(getParam<Real>("factor")),
    _offset(getParam<Real>("offset"))
{
}
