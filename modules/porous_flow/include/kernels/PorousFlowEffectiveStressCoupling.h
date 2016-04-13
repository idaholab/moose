/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef POROUSFLOWEFFECTIVESTRESSCOUPLING_H
#define POROUSFLOWEFFECTIVESTRESSCOUPLING_H

#include "Kernel.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowEffectiveStressCoupling;

template<>
InputParameters validParams<PorousFlowEffectiveStressCoupling>();

/**
 * PorousFlowEffectiveStressCoupling computes -coefficient*effective_porepressure*grad_test[component]
 */
class PorousFlowEffectiveStressCoupling : public Kernel
{
public:

  PorousFlowEffectiveStressCoupling(const InputParameters & parameters);

 protected:

  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

 private:

  /// The Porous-Flow dictator that holds global info about the simulation
  const PorousFlowDictator & _dictator_UO;

  /// Biot coefficient
  const Real _coefficient;

  unsigned int _component;

  /// effective porepressure
  const MaterialProperty<Real> & _pf;

  /// d(effective porepressure)/(d porflow variable)
  const MaterialProperty<std::vector<Real> > & _dpf_dvar;
};

#endif //POROUSFLOWEFFECTIVESTRESSCOUPLING_H
