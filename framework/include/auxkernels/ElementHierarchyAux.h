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
 * An aux kernel object which stores the element hierarchy
 * in an aux variable.
 */

class ElementHierarchyAux : public AuxKernel
{

public:
  static InputParameters validParams();
  ElementHierarchyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
};
