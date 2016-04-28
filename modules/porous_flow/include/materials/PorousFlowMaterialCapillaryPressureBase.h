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
  /// Saturation material property at the nodes
  const MaterialProperty<std::vector<Real> > & _saturation_nodal;
  /// Saturation material property at the qps
  const MaterialProperty<std::vector<Real> > & _saturation_qp;
  /// Capillary pressure material property at the nodes
  MaterialProperty<Real> & _capillary_pressure_nodal;
  /// Derivative of capillary pressure at the nodes wrt phase saturation
  MaterialProperty<Real> & _dcapillary_pressure_nodal_ds;
  /// Second derivative of capillary pressure at the nodes wrt phase saturation
  MaterialProperty<Real> & _d2capillary_pressure_nodal_ds2;
  /// Capillary pressure material property at the qps
  MaterialProperty<Real> & _capillary_pressure_qp;
  /// Derivative of capillary pressure at the qps wrt phase saturation
  MaterialProperty<Real> & _dcapillary_pressure_qp_ds;
  /// Second derivative of capillary pressure at the qps wrt phase saturation
  MaterialProperty<Real> & _d2capillary_pressure_qp_ds2;
};

#endif //POROUSFLOWMATERIALCAPILLARYPRESSUREBASE_H
