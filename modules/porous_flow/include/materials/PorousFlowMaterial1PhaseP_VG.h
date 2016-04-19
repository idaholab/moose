/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef POROUSFLOWMATERIAL1PHASEP_VG_H
#define POROUSFLOWMATERIAL1PHASEP_VG_H

#include "PorousFlowStateBase.h"
#include "PorousFlowCapillaryVG.h"

//Forward Declarations
class PorousFlowMaterial1PhaseP_VG;

template<>
InputParameters validParams<PorousFlowMaterial1PhaseP_VG>();

/**
 * Material designed to calculate fluid-phase porepressure and saturation
 * for the single-phase situation, assuming a van-Genuchten capillary suction function
 */
class PorousFlowMaterial1PhaseP_VG : public PorousFlowStateBase
{
public:
  PorousFlowMaterial1PhaseP_VG(const InputParameters & parameters);

protected:
  
  virtual void initQpStatefulProperties();
  virtual void computeQpProperties();

  /// number of phases (=1 for this class)
  const unsigned int _num_ph;
  /// van-Genuchten alpha parameter
  const Real _al;
  /// van-Genuchten m parameter
  const Real _m;
  /// Nodal value of porepressure of the fluid phase
  const VariableValue & _porepressure_nodal_var;
  /// Quadpoint value of porepressure of the fluid phase
  const VariableValue & _porepressure_qp_var;
  /// Gradient(_porepressure at quadpoints)
  const VariableGradient & _gradp_qp_var;
  /// Moose variable number of the porepressure
  const unsigned int _porepressure_varnum;

 private:
  void buildQpPPSS();
};

#endif //POROUSFLOWMATERIAL1PHASEP_VG_H
