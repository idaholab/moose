/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSPenalizedNormalFlowBC.h"

template<>
InputParameters validParams<NSPenalizedNormalFlowBC>()
{
  InputParameters params = validParams<NSIntegratedBC>();

  // Required parameters
  params.addRequiredParam<Real>("penalty", "The penalty parameter, some (large) value.");

  // Parameters with default values
  params.addParam<Real>("specified_udotn", 0., "The desired value of u.n.");

  return params;
}



NSPenalizedNormalFlowBC::NSPenalizedNormalFlowBC(const std::string & name, InputParameters parameters)
    : NSIntegratedBC(name, parameters),

      // Required parameters
      _penalty(getParam<Real>("penalty")),
      _specified_udotn(getParam<Real>("specified_udotn"))
{
}




Real NSPenalizedNormalFlowBC::computeQpResidual()
{
  RealVectorValue vel(_u_vel[_qp], _v_vel[_qp], _w_vel[_qp]);

  Real residual = _penalty * ((vel*_normals[_qp]) - _specified_udotn) * _test[_i][_qp];
  // Moose::out << "residual[" << _qp << "]=" << residual << std::endl;

  return residual;
}




Real NSPenalizedNormalFlowBC::computeQpJacobian()
{
  // TODO
  return 0.;
}




Real NSPenalizedNormalFlowBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // TODO
  return 0.;
}
