/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "INSMass.h"

template<>
InputParameters validParams<INSMass>()
{
  InputParameters params = validParams<Kernel>();
  
  // Coupled variables
  params.addRequiredCoupledVar("u", "x-velocity");
  params.addCoupledVar("v", 0, "y-velocity"); // only required in 2D and 3D
  params.addCoupledVar("w", 0, "z-velocity"); // only required in 3D
  params.addRequiredCoupledVar("p", "pressure");

  MooseEnum coord_types("XYZ RZ RSPHERICAL");
  params.addParam<MooseEnum>("coord_type", coord_types, "Coordinate system types. Choices are: " + coord_types.getRawNames());
  return params;
}



INSMass::INSMass(const InputParameters & parameters) :
  Kernel(parameters),

  _coord_type_set(false),

  // Velocities (important for non-cartesian coordinates)
  _u_vel(coupledValue("u")),
  _v_vel(coupledValue("v")),
  _w_vel(coupledValue("w")),

  // Gradients
  _grad_u_vel(coupledGradient("u")),
  _grad_v_vel(coupledGradient("v")),
  _grad_w_vel(coupledGradient("w")),

  // Variable numberings
  _u_vel_var_number(coupled("u")),
  _v_vel_var_number(coupled("v")),
  _w_vel_var_number(coupled("w")),
  _p_var_number(coupled("p"))

{
}

void INSMass::setGeometryParameter(const InputParameters & params,
                                    INSMass::COORD_TYPE & coord_type)
{
  if (params.isParamSetByUser("coord_type"))
  {
    coord_type = INSMass::COORD_TYPE(int(params.get<MooseEnum>("coord_type")));
  }
  else
  {
    coord_type = INSMass::XYZ;
  }
}

void INSMass::computeResidual()
{
  if (!_coord_type_set)
  {
    setGeometryParameter(_pars, _coord_type);
    _coord_type_set = true;
  }

  Kernel::computeResidual();
}

Real INSMass::computeQpResidual()
{
  // (div u) * q
  // Note: we (arbitrarily) multilply this term by -1 so that it matches the -p(div v)
  // term in the momentum equation.  Not sure if that is really important?
  switch(_coord_type)
  {
    case INSMass::XYZ:
      return -(_grad_u_vel[_qp](0) + _grad_v_vel[_qp](1) + _grad_w_vel[_qp](2)) * _test[_i][_qp];
    case INSMass::RZ:
      return -(_grad_u_vel[_qp](0) + _u_vel[_qp] / _q_point[_qp](0) + _grad_v_vel[_qp](1)) * _test[_i][_qp];
    case INSMass::RSPHERICAL:
      mooseError("You're looking at flow in a sphere!");
    default:
      mooseError("coord_type doesn't match XYZ, RZ, or RSPHERICAL");
  }
}




Real INSMass::computeQpJacobian()
{
  // Derivative wrt to p is zero
  return 0;
}




Real INSMass::computeQpOffDiagJacobian(unsigned jvar)
{
  if (jvar == _u_vel_var_number)
    return -_grad_phi[_j][_qp](0) * _test[_i][_qp];

  else if (jvar == _v_vel_var_number)
    return -_grad_phi[_j][_qp](1) * _test[_i][_qp];

  else if (jvar == _w_vel_var_number)
    return -_grad_phi[_j][_qp](2) * _test[_i][_qp];
  else
    return 0;
}
