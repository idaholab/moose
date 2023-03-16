//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowVariableBase.h"

class PorousFlowCapillaryPressure;

/**
 * Base material designed to calculate fluid phase porepressure and saturation
 * for the single-phase situation assuming constant effective saturation and
 * porepressure as the nonlinear variable
 */
template <bool is_ad>
class PorousFlow1PhasePTempl : public PorousFlowVariableBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlow1PhasePTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /**
   * Assemble std::vectors of porepressure, saturation and temperature at the quadpoints
   */
  void buildQpPPSS();

  /// Nodal or quadpoint value of porepressure of the fluid phase
  const GenericVariableValue<is_ad> & _porepressure_var;
  /// Gradient(_porepressure at quadpoints)
  const VariableGradient & _gradp_qp_var;
  /// Moose variable number of the porepressure
  const unsigned int _porepressure_varnum;
  /// The PorousFlow variable number of the porepressure
  const unsigned int _p_var_num;
  /// Capillary pressure UserObject
  const PorousFlowCapillaryPressure & _pc_uo;

  using PorousFlowVariableBaseTempl<is_ad>::_dictator;
  using PorousFlowVariableBaseTempl<is_ad>::_porepressure;
  using PorousFlowVariableBaseTempl<is_ad>::_saturation;
  using PorousFlowVariableBaseTempl<is_ad>::_dporepressure_dvar;
  using PorousFlowVariableBaseTempl<is_ad>::_dsaturation_dvar;
  using PorousFlowVariableBaseTempl<is_ad>::_qp;
  using PorousFlowVariableBaseTempl<is_ad>::_num_phases;
  using PorousFlowVariableBaseTempl<is_ad>::_gradp_qp;
  using PorousFlowVariableBaseTempl<is_ad>::_grads_qp;
  using PorousFlowVariableBaseTempl<is_ad>::_dgradp_qp_dgradv;
  using PorousFlowVariableBaseTempl<is_ad>::_dgrads_qp_dgradv;
  using PorousFlowVariableBaseTempl<is_ad>::_dgrads_qp_dv;
  using PorousFlowVariableBaseTempl<is_ad>::_nodal_material;
  using Coupleable::coupled;
  using Coupleable::coupledGradient;
};

typedef PorousFlow1PhasePTempl<false> PorousFlow1PhaseP;
typedef PorousFlow1PhasePTempl<true> ADPorousFlow1PhaseP;
