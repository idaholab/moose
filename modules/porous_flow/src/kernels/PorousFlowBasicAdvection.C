//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowBasicAdvection.h"

registerMooseObject("PorousFlowApp", PorousFlowBasicAdvection);
registerMooseObject("PorousFlowApp", ADPorousFlowBasicAdvection);

template <bool is_ad>
InputParameters
PorousFlowBasicAdvectionTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addParam<unsigned int>("phase", 0, "Use the Darcy velocity of this fluid phase");
  params.addClassDescription(
      "Advective flux of a Variable using the Darcy velocity of the fluid phase");
  return params;
}

template <bool is_ad>
PorousFlowBasicAdvectionTempl<is_ad>::PorousFlowBasicAdvectionTempl(
    const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _dictator(this->template getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _ph(this->template getParam<unsigned int>("phase")),
    _darcy_velocity(this->template getGenericMaterialProperty<std::vector<RealVectorValue>, is_ad>(
        "PorousFlow_darcy_velocity_qp")),
    _ddarcy_velocity_dvar(
        is_ad ? nullptr
              : &this->template getMaterialProperty<std::vector<std::vector<RealVectorValue>>>(
                    "dPorousFlow_darcy_velocity_qp_dvar")),
    _ddarcy_velocity_dgradvar(is_ad ? nullptr
                                    : &this->template getMaterialProperty<
                                          std::vector<std::vector<std::vector<RealVectorValue>>>>(
                                          "dPorousFlow_darcy_velocity_qp_dgradvar"))
{
  if (_ph >= _dictator.numPhases())
    paramError("phase",
               "The Dictator proclaims that the maximum phase index in this simulation is ",
               _dictator.numPhases() - 1,
               " whereas you have used ",
               _ph,
               ". Remember that indexing starts at 0. The Dictator is watching you, to "
               "ensure your wellbeing.");
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowBasicAdvectionTempl<is_ad>::computeQpResidual()
{
  return -_grad_test[_i][_qp] * _darcy_velocity[_qp][_ph] * _u[_qp];
}

template <bool is_ad>
Real
PorousFlowBasicAdvectionTempl<is_ad>::computeQpJacobian()
{
  if constexpr (!is_ad)
  {
    const Real result = -_grad_test[_i][_qp] * _darcy_velocity[_qp][_ph] * _phi[_j][_qp];
    return result + computeQpOffDiagJacobian(_var.number());
  }
  return 0.0;
}

template <bool is_ad>
Real
PorousFlowBasicAdvectionTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  if constexpr (!is_ad)
  {
    if (_dictator.notPorousFlowVariable(jvar))
      return 0.0;

    const unsigned pvar = _dictator.porousFlowVariableNum(jvar);
    Real result =
        -_grad_test[_i][_qp] * (*_ddarcy_velocity_dvar)[_qp][_ph][pvar] * _phi[_j][_qp] * _u[_qp];
    for (unsigned j = 0; j < LIBMESH_DIM; ++j)
      result -= _grad_test[_i][_qp] *
                ((*_ddarcy_velocity_dgradvar)[_qp][_ph][j][pvar] * _grad_phi[_j][_qp](j)) * _u[_qp];

    return result;
  }
  else
  {
    libmesh_ignore(jvar);
    return 0.0;
  }
}

template class PorousFlowBasicAdvectionTempl<false>;
template class PorousFlowBasicAdvectionTempl<true>;
