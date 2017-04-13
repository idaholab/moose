/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RIGIDBODYMODESRZ_H
#define RIGIDBODYMODESRZ_H

#include "NodalUserObject.h"

// Forward Declarations
class RigidBodyModesRZ;

template <>
InputParameters validParams<RigidBodyModesRZ>();

class RigidBodyModesRZ : public NodalUserObject
{
public:
  RigidBodyModesRZ(const InputParameters & parameters);

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
  unsigned int _disp_r_i;
  unsigned int _disp_z_i;
};

#endif
