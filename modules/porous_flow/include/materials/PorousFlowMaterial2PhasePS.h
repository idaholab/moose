/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef PORFLOWMATERIAL2PHASEPS_H
#define PORFLOWMATERIAL2PHASEPS_H

#include "DerivativeMaterialInterface.h"
#include "Material.h"

#include "PorousFlowDictator.h"

//Forward Declarations
class PorousFlowMaterial2PhasePS;

template<>
InputParameters validParams<PorousFlowMaterial2PhasePS>();

/**
 * Material designed to calculate fluid-phase porepressures at nodes
 */
class PorousFlowMaterial2PhasePS : public DerivativeMaterialInterface<Material>
{
public:
  PorousFlowMaterial2PhasePS(const InputParameters & parameters);

protected:

  /// Nodal value of porepressure of the zero phase (eg, the gas phase)
  const VariableValue & _phase0_porepressure;

  /// Quadpoint value of porepressure of the zero phase (eg, the gas phase)
  const VariableValue & _phase0_qp_porepressure;

  /// Gradient(phase0_porepressure)
  const VariableGradient & _phase0_gradp;

  /// Moose variable number of the phase0 porepressure
  const unsigned int _phase0_porepressure_varnum;

  /// Nodal value of saturation of the one phase (eg, the water phase)
  const VariableValue & _phase1_saturation;

  /// Quadpoint value of saturation of the one phase (eg, the water phase)
  const VariableValue & _phase1_qp_saturation;

  /// Gradient(phase1_saturation)
  const VariableGradient & _phase1_grads;

  /// Moose variable number of the phase1 saturation
  const unsigned int _phase1_saturation_varnum;

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

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

 private:
  void buildQpPPSS();
};

#endif //PORFLOWMATERIAL2PHASEPS_H
