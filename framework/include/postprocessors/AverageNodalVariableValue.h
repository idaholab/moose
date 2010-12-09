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

#ifndef AVERAGENODALVARIABLEVALUE_H
#define AVERAGENODALVARIABLEVALUE_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class AverageNodalVariableValue;

template<>
InputParameters validParams<AverageNodalVariableValue>();

class AverageNodalVariableValue : public GeneralPostprocessor
{
public:
  AverageNodalVariableValue(const std::string & name, InputParameters parameters);
  
  virtual void initialize() {}
  
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();

protected:
  Mesh & _mesh;
  std::string _var_name;
  unsigned int _nodesetid;

  std::vector<unsigned int> _node_ids;
};

#endif //PRINTDT_H
