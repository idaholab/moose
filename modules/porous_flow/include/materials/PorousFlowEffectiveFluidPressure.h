//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowMaterialVectorBase.h"

/**
 * Material designed to calculate the effective fluid pressure
 * that can be used in the mechanical effective-stress calculations
 * and other similar places.  This class computes
 * effective fluid pressure = sum_{phases}Saturation_{phase}*Porepressure_{phase}
 */
class PorousFlowEffectiveFluidPressure : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowEffectiveFluidPressure(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Quadpoint or nodal porepressure of each phase
  const MaterialProperty<std::vector<Real>> & _porepressure;

  /// d(porepressure)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dporepressure_dvar;

  /// Quadpoint or nodal saturation of each phase
  const MaterialProperty<std::vector<Real>> & _saturation;

  /// d(saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dsaturation_dvar;

  /// Computed effective fluid pressure (at quadpoints or nodes)
  MaterialProperty<Real> & _pf;

  /// d(_pf)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real>> & _dpf_dvar;
};
