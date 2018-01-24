//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELEMENTVARIABLEPOSTPROCESSOR_H
#define ELEMENTVARIABLEPOSTPROCESSOR_H

#include "ElementPostprocessor.h"
#include "MooseVariableInterface.h"

// Forward Declarations
class ElementVariablePostprocessor;

template <>
InputParameters validParams<ElementVariablePostprocessor>();

class ElementVariablePostprocessor : public ElementPostprocessor, public MooseVariableInterface
{
public:
  ElementVariablePostprocessor(const InputParameters & parameters);

  virtual void execute() override;

protected:
  /// This is what derived classes should override to do something on every quadrature point on every element
  virtual void computeQpValue() = 0;

  /// Holds the solution at current quadrature points
  const VariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const VariableGradient & _grad_u;

  /// Holds the solution derivative at the current quadrature points
  const VariableValue & _u_dot;

  /// The current quadrature point
  unsigned int _qp;
};

#endif
