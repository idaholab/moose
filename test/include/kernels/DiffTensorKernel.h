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
#ifndef DIFFTENSORKERNEL_H
#define DIFFTENSORKERNEL_H

#include "Kernel.h"
#include "MooseParsedVectorFunction.h"
#include "MaterialProperty.h"

// Forward Declaration
class DiffTensorKernel;

template <>
InputParameters validParams<DiffTensorKernel>();

/**
 * A Kernel for Testing ParsedVectorFunction
 */
class DiffTensorKernel : public Kernel
{
public:
  /** Class constructor */
  DiffTensorKernel(const InputParameters & parameters);

  /** Class destructor */
  virtual ~DiffTensorKernel() {}

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  /// A vector function containing the components of k for the tensor
  Function & _k_comp;

private:
  /** Compute the k Tensor from the vector function input */
  RealTensorValue computeConductivity(Real t, const Point & pt);
};

#endif // DIFFTENSORKERNEL_H
