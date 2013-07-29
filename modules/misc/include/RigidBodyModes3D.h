/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef RIGIDBODYMODES3D_H
#define RIGIDBODYMODES3D_H

#include "NodalUserObject.h"

//Forward Declarations
class RigidBodyModes3D;

template<>
InputParameters validParams<RigidBodyModes3D>();

class RigidBodyModes3D :
  public NodalUserObject
{
public:
  RigidBodyModes3D(const std::string & name, InputParameters parameters);

  /**
   * This function will get called on each node.
   */
  virtual void execute();

  virtual void initialize(){};
  virtual void threadJoin(const UserObject&){};
  virtual void finalize();

protected:
  std::string               _subspace_name;
  std::vector<unsigned int> _subspace_indices;
  std::set<std::string>     _modes;
  unsigned int _disp_x_i;
  unsigned int _disp_y_i;
  unsigned int _disp_z_i;
};

#endif
