/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef TORQUEREACTION_H
#define TORQUEREACTION_H

#include "NodalPostprocessor.h"

// Forward Declarations
class TorqueReaction;
class AuxiliarySystem;

template <>
InputParameters validParams<TorqueReaction>();

/*
 * TorqueReaction calculates the torque in 2D and 3D about a user-specified
 * axis of rotation centered at a user-specied origin. The default origin is the
 * global coordinate system origin: (0.0,0.0,0.0).
 *
 * TorqueReaction takes a scalar approach to calculating the sum of the
 * acting torques by projecting both the reaction force and the position vector
 * (the coordinates of the node upon which the force is applied) onto the axis of
 * rotation and applying the Pythagorean theorem, as in a statics course.  This
 * scalar approach allows the postprocessor to accept any axis of rotation direction.
 *
 * TorqueReaction is similar to TorqueReaction in SolidMechanics but does
 * not replace the TorqueReaction postprocessor; different assumptions were used
 * to derive the SolidMechanics TorqueReaction postprocessor.
 */
class TorqueReaction : public NodalPostprocessor
{
public:
  TorqueReaction(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  void threadJoin(const UserObject & y);

protected:
  AuxiliarySystem & _aux;

  std::vector<const VariableValue *> _react;

  const Point _axis_origin;
  const Point _direction_vector;

  unsigned int _nrt;

  Real _sum;
};

#endif // TORQUEREACTION_H
