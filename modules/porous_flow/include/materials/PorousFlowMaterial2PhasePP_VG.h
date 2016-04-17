/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIAL2PHASEPP_VG_H
#define PORFLOWMATERIAL2PHASEPP_VG_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"
#include "PorousFlowCapillaryVG.h"

//Forward Declarations
class PorousFlowMaterial2PhasePP_VG;

template<>
InputParameters validParams<PorousFlowMaterial2PhasePP_VG>();

/**
 * Material designed to calculate fluid-phase porepressures and saturations at nodes and quadpoints
 */
class PorousFlowMaterial2PhasePP_VG : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterial2PhasePP_VG(const InputParameters & parameters);

protected:

  /// vanGenuchten alpha
  const Real _al;

  /// vanGenuchten m
  const Real _m;

  /// Nodal value of porepressure of the zero phase (eg, the water phase)
  const VariableValue & _phase0_porepressure;

  /// Quadpoint value of porepressure of the zero phase (eg, the water phase)
  const VariableValue & _phase0_qp_porepressure;

  /// Gradient(phase0_porepressure)
  const VariableGradient & _phase0_gradp;

  /// Moose variable number of the phase0 porepressure
  const unsigned int _phase0_porepressure_varnum;

  /// Nodal value of porepressure of the one phase (eg, the gas phase)
  const VariableValue & _phase1_porepressure;

  /// Quadpoint value of porepressure of the one phase (eg, the gas phase)
  const VariableValue & _phase1_qp_porepressure;

  /// Gradient(phase1_porepressure)
  const VariableGradient & _phase1_gradp;

  /// Moose variable number of the phase1 porepressure
  const unsigned int _phase1_porepressure_varnum;

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _porflow_name_UO;

  /// nodal porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure;

  /// old value of nodal porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure_old;

  /// quadpoint porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure_qp;

  /// grad(p)
  MaterialProperty<std::vector<RealGradient> > & _gradp;

  /// d(nodal porepressure)/d(nodal porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_dvar;

  /// d(quadpoint porepressure)/d(quadpoint porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_qp_dvar;

  /// d(grad porepressure)/d(grad porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dgradp_dgradv;

  /// d(grad porepressure)/d(porflow variable)
  MaterialProperty<std::vector<std::vector<RealGradient> > > & _dgradp_dv;

  /// nodal saturation of the phases
  MaterialProperty<std::vector<Real> > & _saturation;

  /// old value of nodal saturation of the phases
  MaterialProperty<std::vector<Real> > & _saturation_old;

  /// quadpoint saturation of the phases
  MaterialProperty<std::vector<Real> > & _saturation_qp;

  /// grad(s)
  MaterialProperty<std::vector<RealGradient> > & _grads;

  /// d(nodal saturation)/d(nodal porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_dvar;

  /// d(quadpoint saturation)/d(quadpoint porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_qp_dvar;

  /// d(grad saturation)/d(grad porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dgrads_dgradv;

  /// d(grad saturation)/d(porflow variable)
  MaterialProperty<std::vector<std::vector<RealGradient> > > & _dgrads_dv;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

 private:
  void buildQpPPSS();
};

#endif //PORFLOWMATERIAL2PHASEPP_VG_H
