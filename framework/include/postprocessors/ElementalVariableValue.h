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

#ifndef ELEMENTALVARIABLEVALUE_H
#define ELEMENTALVARIABLEVALUE_H

#include "GeneralPostprocessor.h"
// libMesh
#include "elem.h"

class MooseMesh;

//Forward Declarations
class ElementalVariableValue;

template<>
InputParameters validParams<ElementalVariableValue>();

class ElementalVariableValue : public GeneralPostprocessor
{
public:
  ElementalVariableValue(const std::string & name, InputParameters parameters);

  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the degrees of freedom in the system.
   */
  virtual Real getValue();

protected:
  MooseMesh & _mesh;
  std::string _var_name;
  Elem * _element;
};

#endif //ELEMENTALVARIABLEVALUE_H
