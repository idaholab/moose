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

#ifndef ARRAYDIFFUSION_H
#define ARRAYDIFFUSION_H

#include "ArrayKernel.h"

class ArrayDiffusion;

template<>
InputParameters validParams<ArrayDiffusion>();

/**
 * This kernel implements the Laplacian operator:
 * $\nabla u \cdot \nabla \phi_i$
 */
class ArrayDiffusion : public ArrayKernel
{
public:
  ArrayDiffusion(const InputParameters & parameters);

protected:
  virtual void computeQpResidual() override;

  virtual void computeQpJacobian() override;
};


#endif /* ARRAYDIFFUSION_H */
