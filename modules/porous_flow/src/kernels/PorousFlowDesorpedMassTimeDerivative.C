//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowDesorpedMassTimeDerivative.h"

#include "MooseVariable.h"

#include "libmesh/quadrature.h"

registerMooseObject("PorousFlowApp", PorousFlowDesorpedMassTimeDerivative);
registerMooseObject("PorousFlowApp", ADPorousFlowDesorpedMassTimeDerivative);

template <bool is_ad>
InputParameters
PorousFlowDesorpedMassTimeDerivativeTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";
  params.addRequiredParam<UserObjectName>(
      "PorousFlowDictator", "The UserObject that holds the list of PorousFlow variable names.");
  params.addRequiredCoupledVar(
      "conc_var", "The variable that represents the concentration of desorped species");
  params.addClassDescription("Desorped component mass derivative wrt time.");
  return params;
}

template <bool is_ad>
PorousFlowDesorpedMassTimeDerivativeTempl<is_ad>::PorousFlowDesorpedMassTimeDerivativeTempl(
    const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _dictator(this->template getUserObject<PorousFlowDictator>("PorousFlowDictator")),
    _conc_var_number(this->coupled("conc_var")),
    _conc(this->template coupledGenericValue<is_ad>("conc_var")),
    _conc_old(this->coupledValueOld("conc_var")),
    _porosity(this->template getGenericMaterialProperty<Real, is_ad>("PorousFlow_porosity_qp")),
    _porosity_old(this->template getMaterialPropertyOld<Real>("PorousFlow_porosity_qp")),
    _dporosity_dvar(is_ad ? nullptr
                          : &this->template getMaterialProperty<std::vector<Real>>(
                                "dPorousFlow_porosity_qp_dvar")),
    _dporosity_dgradvar(is_ad ? nullptr
                              : &this->template getMaterialProperty<std::vector<RealGradient>>(
                                    "dPorousFlow_porosity_qp_dgradvar"))
{
}

template <bool is_ad>
GenericReal<is_ad>
PorousFlowDesorpedMassTimeDerivativeTempl<is_ad>::computeQpResidual()
{
  GenericReal<is_ad> c = (1.0 - _porosity[_qp]) * _conc[_qp];
  Real c_old = (1.0 - _porosity_old[_qp]) * _conc_old[_qp];
  return _test[_i][_qp] * (c - c_old) / this->_dt;
}

template <bool is_ad>
Real
PorousFlowDesorpedMassTimeDerivativeTempl<is_ad>::computeQpJacobian()
{
  return computeQpJac(_var.number());
}

template <bool is_ad>
Real
PorousFlowDesorpedMassTimeDerivativeTempl<is_ad>::computeQpOffDiagJacobian(unsigned int jvar)
{
  return computeQpJac(jvar);
}

template <bool is_ad>
Real
PorousFlowDesorpedMassTimeDerivativeTempl<is_ad>::computeQpJac(unsigned int jvar)
{
  if constexpr (!is_ad)
  {
    Real deriv = 0.0;

    if (jvar == _conc_var_number)
      deriv = (1.0 - this->_porosity[this->_qp]) * this->_phi[this->_j][this->_qp];

    if (_dictator.notPorousFlowVariable(jvar))
      return this->_test[this->_i][this->_qp] * deriv / this->_dt;
    const unsigned int pvar = _dictator.porousFlowVariableNum(jvar);

    deriv -= (*_dporosity_dgradvar)[this->_qp][pvar] * this->_grad_phi[this->_j][this->_qp] *
             this->_conc[this->_qp];
    deriv -= (*_dporosity_dvar)[this->_qp][pvar] * this->_phi[this->_j][this->_qp] *
             this->_conc[this->_qp];

    return this->_test[this->_i][this->_qp] * deriv / this->_dt;
  }
  else
    libmesh_ignore(jvar);
  return 0.0;
}

template class PorousFlowDesorpedMassTimeDerivativeTempl<false>;
template class PorousFlowDesorpedMassTimeDerivativeTempl<true>;
