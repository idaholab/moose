#pragma once

#include "Kernel.h"

class MaterialPropertyDependencyKernel : public Kernel
{
public:
  static InputParameters validParams();

  MaterialPropertyDependencyKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  const MaterialProperty<Real> & _prop;
  const Real _dprop_dvar;
};
