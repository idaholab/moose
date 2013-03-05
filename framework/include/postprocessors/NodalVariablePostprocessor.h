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

#ifndef NODALVARIABLEPOSTPROCESSOR_H
#define NODALVARIABLEPOSTPROCESSOR_H

#include "NodalPostprocessor.h"

class MooseVariable;

//Forward Declarations
class NodalVariablePostprocessor;

template<>
InputParameters validParams<NodalVariablePostprocessor>();

class NodalVariablePostprocessor :
  public NodalPostprocessor,
  public MooseVariableInterface
{
public:
  NodalVariablePostprocessor(const std::string & name, InputParameters parameters);

protected:
  MooseVariable & _var;

  /// Holds the solution at current quadrature points
  VariableValue & _u;
};

#endif
