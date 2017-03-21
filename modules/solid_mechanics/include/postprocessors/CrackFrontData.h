/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef CRACKFRONTDATA_H
#define CRACKFRONTDATA_H

#include "GeneralPostprocessor.h"
#include "CrackFrontDefinition.h"
// libMesh
#include "libmesh/node.h"

class MooseMesh;

// Forward Declarations
class CrackFrontData;

template <>
InputParameters validParams<CrackFrontData>();

class CrackFrontData : public GeneralPostprocessor
{
public:
  CrackFrontData(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();

protected:
  const CrackFrontDefinition * const _crack_front_definition;
  const unsigned int _crack_front_point_index;
  const Node * _crack_front_node;
  MooseMesh & _mesh;
  std::string _var_name;
  const Real _scale_factor;
};

#endif // CRACKFRONTDATA_H
