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

#ifndef CRACKFRONTDATA_H
#define CRACKFRONTDATA_H

#include "GeneralPostprocessor.h"
#include "CrackFrontDefinition.h"
// libMesh
#include "libmesh/node.h"

class MooseMesh;

//Forward Declarations
class CrackFrontData;

template<>
InputParameters validParams<CrackFrontData>();

class CrackFrontData : public GeneralPostprocessor
{
public:
  CrackFrontData(const std::string & name, InputParameters parameters);

  virtual void initialize();
  virtual void execute() {}


  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();

protected:
  const CrackFrontDefinition * const _crack_front_definition;
  const unsigned int _crack_front_node_index;
  const Node * _crack_front_node;
  MooseMesh & _mesh;
  std::string _var_name;
  const Real _scale_factor;
};

#endif //CRACKFRONTDATA_H
