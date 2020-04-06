//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernelBasePD.h"

class PeridynamicsMesh;

/**
 * Aux Kernel class to output the offset of boundary nodes from initial FE mesh
 */
class BoundaryOffsetPD : public AuxKernelBasePD
{
public:
  static InputParameters validParams();

  BoundaryOffsetPD(const InputParameters & parameters);

protected:
  Real computeValue() override;
};
