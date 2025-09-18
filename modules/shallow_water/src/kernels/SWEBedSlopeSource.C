//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEBedSlopeSource.h"
#include "Function.h"

registerMooseObject("ShallowWaterApp", SWEBedSlopeSource);

InputParameters
SWEBedSlopeSource::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("Bed slope source for SWE momentum: -g h grad(b) component.");
  params.addRequiredCoupledVar("gravity", "Scalar gravity field g");
  params.addRequiredCoupledVar("h", "Conserved variable: h");
  params.addRequiredParam<FunctionName>("bed", "Bed elevation function b(x,y)");
  MooseEnum dir("x=0 y=1");
  params.addParam<MooseEnum>("direction", dir, "Direction of momentum component (x or y)");
  return params;
}

SWEBedSlopeSource::SWEBedSlopeSource(const InputParameters & parameters)
  : Kernel(parameters),
    _g(coupledValue("gravity")),
    _h_var(coupled("h")),
    _h(coupledValue("h")),
    _bed(getFunction("bed")),
    _dir(getParam<MooseEnum>("direction"))
{
}

SWEBedSlopeSource::~SWEBedSlopeSource() {}

Real
SWEBedSlopeSource::computeQpResidual()
{
  const auto gradb = _bed.gradient(_t, _q_point[_qp]);
  const Real db = (_dir == 0 ? gradb(0) : gradb(1));
  const Real S = -_g[_qp] * _h[_qp] * db;
  return S * _test[_i][_qp];
}

Real
SWEBedSlopeSource::computeQpJacobian()
{
  // derivative w.r.t. this variable (hu or hv) is zero for this source
  return 0.0;
}

Real
SWEBedSlopeSource::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (jvar == _h_var)
  {
    const auto gradb = _bed.gradient(_t, _q_point[_qp]);
    const Real db = (_dir == 0 ? gradb(0) : gradb(1));
    return (-_g[_qp] * db) * _phi[_j][_qp] * _test[_i][_qp];
  }
  return 0.0;
}
