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

#ifndef RIGIDBODYMODESRZ_H
#define RIGIDBODYMODESRZ_H

#include "NodalUserObject.h"

//Forward Declarations
class RigidBodyModesRZ;

template<>
InputParameters validParams<RigidBodyModesRZ>();

class RigidBodyModesRZ :
  public NodalUserObject
{
public:
  RigidBodyModesRZ(const std::string & name, InputParameters parameters);

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
  unsigned int _disp_r_i;
  unsigned int _disp_z_i;
};

#endif
