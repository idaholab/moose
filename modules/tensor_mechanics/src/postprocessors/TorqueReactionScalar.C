/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "TorqueReactionScalar.h"

template<>
InputParameters validParams<TorqueReactionScalar>()
{
  InputParameters params = validParams<NodalPostprocessor>();
  params.addParam<AuxVariableName>("react_x", "The x reaction variable");
  params.addParam<AuxVariableName>("react_y", "The y reaction variable");
  params.addParam<AuxVariableName>("react_z", "The z reaction variable");

  params.addParam<RealVectorValue>("axis_origin", Point(), "Origin of the axis of rotation used to calculate the torque");
  params.addRequiredParam<RealVectorValue>("direction_vector", "The direction vector, originating at (0,0,0), of the axis of rotation about which the calculated torque is applied");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

TorqueReactionScalar::TorqueReactionScalar(const InputParameters & parameters) :
    NodalPostprocessor(parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _react_x_var(_aux.getVariable(_tid, parameters.get<AuxVariableName>("react_x"))),
    _react_y_var(_aux.getVariable(_tid, parameters.get<AuxVariableName>("react_y"))),
    _react_z_var(_aux.getVariable(_tid, parameters.get<AuxVariableName>("react_z"))),
    _react_x(_react_x_var.nodalSln()),
    _react_y(_react_y_var.nodalSln()),
    _react_z(_react_z_var.nodalSln()),
    _axis_origin(getParam<RealVectorValue>("axis_origin")),
    _direction_vector(getParam<RealVectorValue>("direction_vector"))
{
}

void
TorqueReactionScalar::initialize()
{
  _sum = 0.0;
}

void
TorqueReactionScalar::execute()
{
  // Tranform the node coordinates into the coordinate system specified by the user
  RealVectorValue position = (*_current_node) - _axis_origin;

  // Determine the projection of the transformed (adjusted) node position vector onto the direction vector
  Real position_projection = position * _direction_vector;

  // Use Pythagorean theorem to determine the scalar normal (shortest) distance from the axis of the rotation to the node
  Real moment_lever = std::sqrt(position.norm_sq() - position_projection * position_projection);

  // Define the force vector from the reaction force/ residuals from the stress divergence kernel
  Point force(_react_x[_qp], _react_y[_qp], _react_z[_qp]);

  // Calculate the scalar component of the force acting along the axis of rotation direction vector
  Real force_projection = force * _direction_vector;

  _sum += moment_lever * force_projection;
}

Real
TorqueReactionScalar::getValue()
{
  gatherSum(_sum);

  return _sum;
}

void
TorqueReactionScalar::threadJoin(const UserObject & y)
{
  const TorqueReactionScalar & pps = static_cast<const TorqueReactionScalar &>(y);
  _sum += pps._sum;
}
