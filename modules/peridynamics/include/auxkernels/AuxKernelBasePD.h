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

class AuxKernelBasePD;
class PeridynamicsMesh;

template <>
InputParameters validParams<AuxKernelBasePD>();

/**
 * Peridynamic Aux Kernel base class
 */
class AuxKernelBasePD : public AuxKernel
{
public:
  AuxKernelBasePD(const InputParameters & parameters);

protected:
  /// Reference to peridynamic mesh object
  PeridynamicsMesh & _pdmesh;

  /// Problem dimension
  const unsigned int _dim;
};
