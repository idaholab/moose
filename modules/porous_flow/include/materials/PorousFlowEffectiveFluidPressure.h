//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
template <bool is_ad>
class PorousFlowEffectiveFluidPressureTempl : public PorousFlowMaterialVectorBase
{
public:
  static InputParameters validParams();

  PorousFlowEffectiveFluidPressureTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Quadpoint or nodal porepressure of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _porepressure;

  /// d(porepressure)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dporepressure_dvar;

  /// Quadpoint or nodal saturation of each phase
  const GenericMaterialProperty<std::vector<Real>, is_ad> & _saturation;

  /// d(saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> * const _dsaturation_dvar;

  /// Computed effective fluid pressure (at quadpoints or nodes)
  GenericMaterialProperty<Real, is_ad> & _pf;

  /// d(_pf)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real>> * const _dpf_dvar;
};

typedef PorousFlowEffectiveFluidPressureTempl<false> PorousFlowEffectiveFluidPressure;
typedef PorousFlowEffectiveFluidPressureTempl<true> ADPorousFlowEffectiveFluidPressure;
