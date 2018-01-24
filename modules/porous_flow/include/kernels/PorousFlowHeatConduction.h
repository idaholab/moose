/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWHEATCONDUCTION_H
#define POROUSFLOWHEATCONDUCTION_H

#include "Kernel.h"
#include "PorousFlowDictator.h"

// Forward Declarations
class PorousFlowHeatConduction;

template <>
InputParameters validParams<PorousFlowHeatConduction>();

/**
 * Kernel = grad(test) * thermal_conductivity * grad(temperature)
 */
class PorousFlowHeatConduction : public Kernel
{
public:
  PorousFlowHeatConduction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// holds info on the PorousFlow variables
  const PorousFlowDictator & _dictator;

  /// thermal conductivity at the quadpoints
  const MaterialProperty<RealTensorValue> & _la;

  /// d(thermal conductivity at the quadpoints)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealTensorValue>> & _dla_dvar;

  /// grad(temperature)
  const MaterialProperty<RealGradient> & _grad_t;

  /// d(gradT)/d(PorousFlow variable)
  const MaterialProperty<std::vector<RealGradient>> & _dgrad_t_dvar;

  /// d(gradT)/d(grad PorousFlow variable)
  const MaterialProperty<std::vector<Real>> & _dgrad_t_dgradvar;
};

#endif // POROUSFLOWHEATCONDUCTION_H
