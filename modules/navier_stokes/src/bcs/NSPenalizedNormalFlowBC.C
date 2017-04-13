/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSPenalizedNormalFlowBC.h"

template <>
InputParameters
validParams<NSPenalizedNormalFlowBC>()
{
  InputParameters params = validParams<NSIntegratedBC>();
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
