//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSPenalizedNormalFlowBC.h"

registerMooseObject("NavierStokesApp", NSPenalizedNormalFlowBC);

InputParameters
NSPenalizedNormalFlowBC::validParams()
{
  InputParameters params = NSIntegratedBC::validParams();
  params.addClassDescription("This class penalizes the the value of u.n on the boundary so that it "
                             "matches some desired value.");
  params.addRequiredParam<Real>("penalty", "The penalty parameter, some (large) value.");
  params.addParam<Real>("specified_udotn", 0., "The desired value of u.n.");
  return params;
}

NSPenalizedNormalFlowBC::NSPenalizedNormalFlowBC(const InputParameters & parameters)
  : NSIntegratedBC(parameters),
    _penalty(getParam<Real>("penalty")),
    _specified_udotn(getParam<Real>("specified_udotn"))
{
}

Real
NSPenalizedNormalFlowBC::computeQpResidual()
{
  const RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);
  return _penalty * ((vel * _normals[_qp]) - _specified_udotn) * _test[_i][_qp];
}

Real
NSPenalizedNormalFlowBC::computeQpJacobian()
{
  // TODO
  return 0.0;
}

Real
NSPenalizedNormalFlowBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // TODO
  return 0.0;
}
