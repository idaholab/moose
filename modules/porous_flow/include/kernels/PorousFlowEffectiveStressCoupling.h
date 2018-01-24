//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POROUSFLOWEFFECTIVESTRESSCOUPLING_H
#define POROUSFLOWEFFECTIVESTRESSCOUPLING_H

#include "Kernel.h"
#include "PorousFlowDictator.h"

// Forward Declarations
class PorousFlowEffectiveStressCoupling;

template <>
InputParameters validParams<PorousFlowEffectiveStressCoupling>();

/**
 * PorousFlowEffectiveStressCoupling computes
 * -coefficient*effective_porepressure*grad_component(test)
 * where component is the spatial component (not
 * a fluid component!)
 */
class PorousFlowEffectiveStressCoupling : public Kernel
{
public:
  PorousFlowEffectiveStressCoupling(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// The Porous-Flow dictator that holds global info about the simulation
  const PorousFlowDictator & _dictator;

  /// Biot coefficient
  const Real _coefficient;

  /// the spatial component
  const unsigned int _component;

  /// effective porepressure
  const MaterialProperty<Real> & _pf;

  /// d(effective porepressure)/(d porflow variable)
  const MaterialProperty<std::vector<Real>> & _dpf_dvar;

  /// Whether an RZ coordinate system is being used
  const bool _rz;
};

#endif // POROUSFLOWEFFECTIVESTRESSCOUPLING_H
