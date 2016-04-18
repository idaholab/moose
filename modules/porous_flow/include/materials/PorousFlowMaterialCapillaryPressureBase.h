/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIALCAPILLARYPRESSUREBASE_H
#define POROUSFLOWMATERIALCAPILLARYPRESSUREBASE_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"
#include "PorousFlowDictator.h"

class PorousFlowMaterialCapillaryPressureBase;

template<>
InputParameters validParams<PorousFlowMaterialCapillaryPressureBase>();

/**
 * Base class for relative permeability materials
 */
class PorousFlowMaterialCapillaryPressureBase : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterialCapillaryPressureBase(const InputParameters & parameters);

protected:

  virtual void computeQpProperties();

  /// Phase number of fluid that this relative permeability relates to
  const unsigned int _phase_num;
  /// The PorousFlowDictator UserObject
  const PorousFlowDictator & _dictator_UO;
  /// Name of (dummy) saturation primary variable
  VariableName _saturation_variable_name;
  /// Saturation material property
  const MaterialProperty<std::vector<Real> > & _saturation;
  /// Capillary pressure material property
  MaterialProperty<Real> & _capillary_pressure;
  /// Derivative of capillary pressure wrt phase saturation
  MaterialProperty<Real> & _dcapillary_pressure_ds;
};

#endif //POROUSFLOWMATERIALCAPILLARYPRESSUREBASE_H
