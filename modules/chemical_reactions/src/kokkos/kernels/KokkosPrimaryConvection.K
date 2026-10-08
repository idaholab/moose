//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosPrimaryConvection.h"

registerKokkosResidualObject("ChemicalReactionsApp", KokkosPrimaryConvection);

InputParameters
KokkosPrimaryConvection::validParams()
{
  InputParameters params = KernelValue::validParams();
  params.addRequiredCoupledVar("p", "Pressure");
  RealVectorValue g(0, 0, 0);
  params.addParam<RealVectorValue>("gravity", g, "Gravity vector (default is (0, 0, 0))");
  params.addClassDescription("Convection of primary species");
  return params;
}

KokkosPrimaryConvection::KokkosPrimaryConvection(const InputParameters & parameters)
  : KernelValue(parameters),
    _cond(getKokkosMaterialProperty<Real>("conductivity")),
    _gravity(getParam<RealVectorValue>("gravity")),
    _density(hasKokkosMaterialProperty<Real>("density") ? getKokkosMaterialProperty<Real>("density")
                                                        : Moose::Kokkos::MaterialProperty<Real>(0)),
    _grad_p(kokkosCoupledGradient("p")),
    _pvar(coupled("p"))
{
}
