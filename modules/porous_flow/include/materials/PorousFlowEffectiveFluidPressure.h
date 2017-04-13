/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWEFFECTIVEFLUIDPRESSURE_H
#define POROUSFLOWEFFECTIVEFLUIDPRESSURE_H

#include "PorousFlowMaterialVectorBase.h"

// Forward Declarations
class PorousFlowEffectiveFluidPressure;

template <>
InputParameters validParams<PorousFlowEffectiveFluidPressure>();

/**
 * Material designed to calculate the effective fluid pressure
 * that can be used in the mechanical effective-stress calculations
 * and other similar places.  This class computes
 * effective fluid pressure = sum_{phases}Saturation_{phase}*Porepressure_{phase}
 */
class PorousFlowEffectiveFluidPressure : public PorousFlowMaterialVectorBase
{
public:
  PorousFlowEffectiveFluidPressure(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// quadpoint or nodal porepressure of each phase
  const MaterialProperty<std::vector<Real>> & _porepressure;

  /// old value of quadpoint or nodal porepressure of each phase
  const MaterialProperty<std::vector<Real>> & _porepressure_old;

  /// d(porepressure)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dporepressure_dvar;

  /// quadpoint or nodal saturation of each phase
  const MaterialProperty<std::vector<Real>> & _saturation;

  /// old value of quadpoint or nodal saturation of each phase
  const MaterialProperty<std::vector<Real>> & _saturation_old;

  /// d(saturation)/d(PorousFlow variable)
  const MaterialProperty<std::vector<std::vector<Real>>> & _dsaturation_dvar;

  /// computed effective fluid pressure (at quadpoints or nodes)
  MaterialProperty<Real> & _pf;

  /// d(_pf)/d(PorousFlow variable)
  MaterialProperty<std::vector<Real>> & _dpf_dvar;
};

#endif // POROUSFLOWEFFECTIVEFLUIDPRESSURE_H
