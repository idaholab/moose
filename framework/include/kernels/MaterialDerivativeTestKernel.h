//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
