/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "CoupledKernelGradBC.h"
#include "Function.h"

template <>
InputParameters
validParams<CoupledKernelGradBC>()
{
  InputParameters params = validParams<IntegratedBC>();
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
