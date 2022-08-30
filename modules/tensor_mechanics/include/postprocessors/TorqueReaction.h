//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalPostprocessor.h"

// Forward Declarations
class AuxiliarySystem;

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
  static InputParameters validParams();

  TorqueReaction(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  virtual void finalize();
  void threadJoin(const UserObject & y);

protected:
  AuxiliarySystem & _aux;

  std::vector<const VariableValue *> _react;

  const Point _axis_origin;
  const Point _direction_vector;

  unsigned int _nrt;

  Real _sum;
};
