//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InternalSideIntegralPostprocessor.h"

#include "libmesh/quadrature.h"

InputParameters
InternalSideIntegralPostprocessor::validParams()
{
  InputParameters params = InternalSidePostprocessor::validParams();
  return params;
}

InternalSideIntegralPostprocessor::InternalSideIntegralPostprocessor(
    const InputParameters & parameters)
  : InternalSidePostprocessor(parameters), _qp(0), _integral_value(0), _qp_integration(true)
{
}

void
InternalSideIntegralPostprocessor::initialize()
{
  _integral_value = 0;
}

void
InternalSideIntegralPostprocessor::execute()
{
  _integral_value += computeIntegral();
}

Real
InternalSideIntegralPostprocessor::getValue()
{

  return _integral_value;
}

void
InternalSideIntegralPostprocessor::threadJoin(const UserObject & y)
{
  const InternalSideIntegralPostprocessor & pps =
      static_cast<const InternalSideIntegralPostprocessor &>(y);
  _integral_value += pps._integral_value;
}

Real
InternalSideIntegralPostprocessor::computeIntegral()
{
  Real sum = 0;
  if (_qp_integration)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      sum += _JxW[_qp] * _coord[_qp] * computeQpIntegral();
  else
  {
    // Finite volume functors integration is over FaceInfo, not quadrature points
    getFaceInfos();

    for (auto & fi : _face_infos)
      sum += fi->faceArea() * fi->faceCoord() * computeFaceInfoIntegral(fi);
  }
  return sum;
}

void
InternalSideIntegralPostprocessor::finalize()
{
  gatherSum(_integral_value);
}
