/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWTESTKERNEL_H
#define POROUSFLOWTESTKERNEL_H

#include "Kernel.h"
#include "DerivativeMaterialInterface.h"

class PorousFlowTestKernel;

template<>
InputParameters validParams<PorousFlowTestKernel>();

class PorousFlowTestKernel : public DerivativeMaterialInterface<Kernel>
{
public:
  PorousFlowTestKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  /// Relative permeability
  const MaterialProperty<Real> & _relative_permeability;
  /// Vector of derivatives of rel permeability
  std::vector<const MaterialProperty<Real> *> _drelative_permeability_dvar;

};

#endif // POROUSFLOWTESTKERNEL_H
