/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWCAPILLARYPRESSUREBASE_H
#define POROUSFLOWCAPILLARYPRESSUREBASE_H

#include "PorousFlowMaterialBase.h"

class PorousFlowCapillaryPressureBase;

template<>
InputParameters validParams<PorousFlowCapillaryPressureBase>();

/**
 * Base class for capillary pressure
 */
class PorousFlowCapillaryPressureBase : public PorousFlowMaterialBase
{
public:
  PorousFlowCapillaryPressureBase(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

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

#endif //POROUSFLOWCAPILLARYPRESSUREBASE_H
