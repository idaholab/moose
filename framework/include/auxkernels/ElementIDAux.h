//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Auxiliary kernel to return the current element ID from the mesh.
 */
class ElementIDAux : public AuxKernel
{
public:
  ElementIDAux(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  virtual Real computeValue();
};
