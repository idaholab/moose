//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlow1PhaseFullySaturated.h"

registerMooseObject("PorousFlowApp", PorousFlow1PhaseFullySaturated);
registerMooseObject("PorousFlowApp", ADPorousFlow1PhaseFullySaturated);

template <bool is_ad>
InputParameters
PorousFlow1PhaseFullySaturatedTempl<is_ad>::validParams()
{
  InputParameters params = PorousFlowVariableBase::validParams();
  params.addRequiredCoupledVar("porepressure",
                               "Variable that represents the porepressure of the single phase");
  params.addClassDescription("This Material is used for the fully saturated single-phase situation "
                             "where porepressure is the primary variable");
  return params;
}

template <bool is_ad>
PorousFlow1PhaseFullySaturatedTempl<is_ad>::PorousFlow1PhaseFullySaturatedTempl(
    const InputParameters & parameters)
  : PorousFlowVariableBaseTempl<is_ad>(parameters),

    _porepressure_var(_nodal_material ? this->template coupledGenericDofValue<is_ad>("porepressure")
                                      : this->template coupledGenericValue<is_ad>("porepressure")),
    _gradp_qp_var(this->template coupledGenericGradient<is_ad>("porepressure")),
    _porepressure_varnum(coupled("porepressure")),
    _p_var_num(_dictator.isPorousFlowVariable(_porepressure_varnum)
                   ? _dictator.porousFlowVariableNum(_porepressure_varnum)
                   : 0)
{
  if (_num_phases != 1)
    mooseError("The Dictator proclaims that the number of phases is ",
               _dictator.numPhases(),
               " whereas PorousFlow1PhaseFullySaturated can only be used for 1-phase simulations."
               " Be aware that the Dictator has noted your mistake.");
}

template <bool is_ad>
void
PorousFlow1PhaseFullySaturatedTempl<is_ad>::initQpStatefulProperties()
{
  PorousFlowVariableBaseTempl<is_ad>::initQpStatefulProperties();
  buildQpPPSS();
}

template <bool is_ad>
void
PorousFlow1PhaseFullySaturatedTempl<is_ad>::computeQpProperties()
{
  // Size vectors correctly and prepare the derivative matrices with zeroes
  PorousFlowVariableBaseTempl<is_ad>::computeQpProperties();

  buildQpPPSS();

  if (!_nodal_material)
    (*_gradp_qp)[_qp][0] = _gradp_qp_var[_qp];

  // _porepressure is only dependent on _porepressure, and its derivative is 1
  if (!is_ad && _dictator.isPorousFlowVariable(_porepressure_varnum))
  {
    // _porepressure is a PorousFlow variable
    (*_dporepressure_dvar)[_qp][0][_p_var_num] = 1.0;
    if (!_nodal_material)
      (*_dgradp_qp_dgradv)[_qp][0][_p_var_num] = 1.0;
  }
}

template <bool is_ad>
void
PorousFlow1PhaseFullySaturatedTempl<is_ad>::buildQpPPSS()
{
  _porepressure[_qp][0] = _porepressure_var[_qp];
  _saturation[_qp][0] = 1.0;
}

template class PorousFlow1PhaseFullySaturatedTempl<false>;
template class PorousFlow1PhaseFullySaturatedTempl<true>;
