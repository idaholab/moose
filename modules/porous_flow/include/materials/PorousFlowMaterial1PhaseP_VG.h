/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIAL1PHASEP_VG_H
#define POROUSFLOWMATERIAL1PHASEP_VG_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"
#include "PorousFlowCapillaryVG.h"

//Forward Declarations
class PorousFlowMaterial1PhaseP_VG;

template<>
InputParameters validParams<PorousFlowMaterial1PhaseP_VG>();

/**
 * Material designed to calculate fluid-phase porepressure and saturation
 * for the single-phase situation, assuming a van-Genuchten capillary suction function
 */
class PorousFlowMaterial1PhaseP_VG : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterial1PhaseP_VG(const InputParameters & parameters);

protected:

  /// number of phases (=1 for this class)
  const unsigned int _num_ph;

  /// van-Genuchten alpha parameter
  const Real _al;

  /// van-Genuchten m parameter
  const Real _m;

  /// Nodal value of porepressure of the fluid phase
  const VariableValue & _porepressure_var;

  /// Quadpoint value of porepressure of the fluid phase
  const VariableValue & _qp_porepressure_var;

  /// Gradient(_porepressure at quadpoints)
  const VariableGradient & _gradp_var;

  /// Moose variable number of the porepressure
  const unsigned int _porepressure_varnum;

  /// The variable names UserObject for the Porous-Flow variables
  const PorousFlowDictator & _porflow_name_UO;

  /// nodal porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure;

  /// old value of nodal porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure_old;

  /// quadpoint porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure_qp;

  /// grad(p) at the quadpoints
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

  /// grad(s) at the quadpoints
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

#endif //POROUSFLOWMATERIAL1PHASEP_VG_H
