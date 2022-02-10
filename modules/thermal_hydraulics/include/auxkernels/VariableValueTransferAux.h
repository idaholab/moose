//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NearestNodeValueAux.h"

class PenetrationLocator;
class NearestNodeLocator;

/**
 * Transfer variable values from a surface of a 2D mesh onto 1D mesh
 */
class VariableValueTransferAux : public AuxKernel
{
public:
  VariableValueTransferAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  PenetrationLocator & _penetration_locator;
  NearestNodeLocator & _nearest_node;

  const NumericVector<Number> * const & _serialized_solution;

  unsigned int _paired_variable;

public:
  static InputParameters validParams();
};
