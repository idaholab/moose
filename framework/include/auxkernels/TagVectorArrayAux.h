//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * The value of a tagged vector for a given node and a given variable is coupled to
 * the current AuxVariable. TagVectorArrayAux returns the coupled nodal value for the user-supplied
 * array variable component
 */
class TagVectorArrayAux : public AuxKernel
{
public:
  static InputParameters validParams();

  TagVectorArrayAux(const InputParameters & parameters);

protected:
  Real computeValue() override;

  /// The result of evaluating the supplied tagged vector at the degrees of freedom corresponding to
  /// the provided array variable
  const ArrayVariableValue & _v;

  /// The array variable component for correctly indexing \p _v
  const unsigned int _component;
};
