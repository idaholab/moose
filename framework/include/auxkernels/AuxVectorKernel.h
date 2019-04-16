//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef AUXVECTORKERNEL_H
#define AUXVECTORKERNEL_H

#include "AuxKernelBase.h"

class AuxVectorKernel;

template <>
InputParameters validParams<AuxVectorKernel>();

/**
 * AuxVectorValue allows for the explicit calculation for vector value variables.
 *
 * Notice that the initialization of _u etc. doesn't consider the nodal case as the
 * the regular AuxKernel class does. Currently, the available vector variables are not
 * continuous at nodes so it is not possible to create a nodal version. Since, the _nodal in the
 * base class is initialized using the variable isNodal() method it is not possible to instantiate
 * a nodal version of this class, thus no additional error checks are needed at this point.
 */
class AuxVectorKernel : public AuxKernelBase<RealVectorValue>
{
public:
  AuxVectorKernel(const InputParameters & parameters);
};

#endif
