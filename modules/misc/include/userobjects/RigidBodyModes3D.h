//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RIGIDBODYMODES3D_H
#define RIGIDBODYMODES3D_H

#include "NodalUserObject.h"

// Forward Declarations
class RigidBodyModes3D;

template <>
InputParameters validParams<RigidBodyModes3D>();

class RigidBodyModes3D : public NodalUserObject
{
public:
  RigidBodyModes3D(const InputParameters & parameters);

  /**
   * This function will get called on each node.
   */
  virtual void execute();

  virtual void initialize(){};
  virtual void threadJoin(const UserObject &){};
  virtual void finalize();

protected:
  std::string _subspace_name;
  std::vector<unsigned int> _subspace_indices;
  std::set<std::string> _modes;
  unsigned int _disp_x_i;
  unsigned int _disp_y_i;
  unsigned int _disp_z_i;
};

#endif
