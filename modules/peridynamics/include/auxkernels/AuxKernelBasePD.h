//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef AUXKERNELBASEPD_H
#define AUXKERNELBASEPD_H

#include "AuxKernel.h"

class AuxKernelBasePD;
class MeshBasePD;

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
  MeshBasePD & _pdmesh;

  /// Problem dimension
  unsigned int _dim;
};

#endif // AUXKERNELBASEPD_H
