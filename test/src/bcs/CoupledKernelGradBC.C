//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledKernelGradBC.h"
#include "Function.h"

registerMooseObject("MooseTestApp", CoupledKernelGradBC);

InputParameters
CoupledKernelGradBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredCoupledVar("var2", "Coupled Variable");
  params.addRequiredParam<std::vector<Real>>("vel", "velocity");
  return params;
}

CoupledKernelGradBC::CoupledKernelGradBC(const InputParameters & parameters)
  : IntegratedBC(parameters), _var2(coupledValue("var2"))
{
  std::vector<Real> a(getParam<std::vector<Real>>("vel"));
  if (a.size() != 2)
  {
    mooseError("ERROR: CoupledKernelGradBC only implemented for 2d, vel is not size 2");
  }
  _beta = RealVectorValue(a[0], a[1]);
}

CoupledKernelGradBC::~CoupledKernelGradBC() {}

Real
CoupledKernelGradBC::computeQpResidual()
{
  return _test[_i][_qp] * ((_beta * _var2[_qp]) * _normals[_qp]);
}
