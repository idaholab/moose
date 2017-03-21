/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
