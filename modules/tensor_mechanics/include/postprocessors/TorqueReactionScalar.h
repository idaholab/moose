/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef TORQUEREACTIONSCALAR_H
#define TORQUEREACTIONSCALAR_H

#include "NodalPostprocessor.h"

//Forward Declarations
class TorqueReactionScalar;
class AuxiliarySystem;

template<>
InputParameters validParams<TorqueReactionScalar>();

/*
 * TorqueReactionScalarScalar calculates the torque in 2D and 3D about a user-specified
 * axis of rotation centered at a user-specied origin. The default origin is the
 * global coordinate system origin: (0.0,0.0,0.0).
 *
 * TorqueReactionScalarScalar takes a scalar approach to calculating the sum of the
 * acting torques by projecting both the reaction force and the position vector
 * (the coordinates of the node upon which the force is applied) onto the axis of
 * rotation and applying the Pythagorean theorem, as in a statics course.  This
 * scalar approach allows the postprocessor to accept any axis of rotation direction.
 *
 * TorqueReactionScalarScalar is similar to TorqueReactionScalar in SolidMechanics but does
 * not replace the TorqueReactionScalar postprocessor; different assumptions were used
 * to derive the SolidMechanics TorqueReactionScalar postprocessor.
 */
class TorqueReactionScalar : public NodalPostprocessor
{
public:
  TorqueReactionScalar(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  void threadJoin(const UserObject & y);

protected:
  AuxiliarySystem & _aux;
  MooseVariable & _react_x_var;
  MooseVariable & _react_y_var;
  MooseVariable & _react_z_var;

  const VariableValue & _react_x;
  const VariableValue & _react_y;
  const VariableValue & _react_z;

  const Point _axis_origin;
  const Point _direction_vector;

  Real _sum;
};

#endif //TORQUEREACTIONSCALAR_H
