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

#ifndef MATERIALDERIVATIVETESTKERNEL_H
#define MATERIALDERIVATIVETESTKERNEL_H

#include "MaterialDerivativeTestKernelBase.h"

class MaterialDerivativeTestKernel;

template <>
InputParameters validParams<MaterialDerivativeTestKernel>();

/**
 * This kernel is used for testing derivatives of a material property.
 */
class MaterialDerivativeTestKernel : public MaterialDerivativeTestKernelBase<Real>
{
public:
  MaterialDerivativeTestKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;
};

#endif /* MATERIALDERIVATIVETESTKERNEL_H */
