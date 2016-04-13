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

#include "PorFlowVarNames.h"

//Forward Declarations
class PorFlowMaterial2PhasePS;

template<>
InputParameters validParams<PorFlowMaterial2PhasePS>();

/**
 * Material designed to calculate fluid-phase porepressures at nodes
 */
class PorFlowMaterial2PhasePS : public DerivativeMaterialInterface<Material>
{
public:
  PorFlowMaterial2PhasePS(const InputParameters & parameters);

protected:

  /// Porepressure of the zero phase (eg, the gas phase)
  const VariableValue & _phase0_porepressure;

  /// Moose variable number of the phase0 porepressure
  const unsigned int _phase0_porepressure_varnum;

  /// Saturation of the one phase (eg, the water phase)
  const VariableValue & _phase1_saturation;

  /// Moose variable number of the phase1 saturation
  const unsigned int _phase1_saturation_varnum;

  /// The variable names UserObject for the Porous-Flow variables
  const PorFlowVarNames & _porflow_name_UO;

  /// porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure;

  /// old value of porepressure of the phases
  MaterialProperty<std::vector<Real> > & _porepressure_old;

  /// d(porepressure)/d(porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dporepressure_dvar;

  /// saturation of the phases
  MaterialProperty<std::vector<Real> > & _saturation;

  /// old value of saturation of the phases
  MaterialProperty<std::vector<Real> > & _saturation_old;

  /// d(porepressure)/d(porflow variable)
  MaterialProperty<std::vector<std::vector<Real> > > & _dsaturation_dvar;

  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

 private:
  void buildQpPPSS();
};

#endif //PORFLOWMATERIAL2PHASEPS_H
