//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VECTORPOSTPROCESSORSCALARKERNEL_H
#define VECTORPOSTPROCESSORSCALARKERNEL_H

#include "ODEKernel.h"

// Forward Declarations
class VectorPostprocessorScalarKernel;

template <>
InputParameters validParams<VectorPostprocessorScalarKernel>();

/**
 *
 */
class VectorPostprocessorScalarKernel : public ODEKernel
{
public:
  VectorPostprocessorScalarKernel(const InputParameters & parameters);
  virtual ~VectorPostprocessorScalarKernel();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const VectorPostprocessorValue & _vpp;

  unsigned int _index;
};

#endif /* VECTORPOSTPROCESSORSCALARKERNEL_H */
