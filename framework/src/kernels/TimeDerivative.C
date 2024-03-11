//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeDerivative.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"

#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", TimeDerivative);
registerMooseObject("MooseApp", VectorTimeDerivative);

template <typename TK>
InputParameters
TimeDerivativeTempl<TK>::validParams()
{
  InputParameters params = TK::validParams();
  if (std::is_same_v<TK, TimeKernel>)
    params.addClassDescription("The time derivative operator with weak form "
                               "$(\\psi_i, k\\frac{\\partial u_h}{\\partial t})$.");
  else if (std::is_same_v<TK, VectorTimeKernel>)
    params.addClassDescription("The vector time derivative operator with weak form "
                               "$(\\vec{\\psi_i}, k\\frac{\\partial \\vec{u_h}}{\\partial t})$.");
  params.addParam<bool>("lumping", false, "True for mass matrix lumping, false otherwise");
  params.addParam<Real>("coeff", 1.0, "The constant coefficient");
  return params;
}

template <typename TK>
TimeDerivativeTempl<TK>::TimeDerivativeTempl(const InputParameters & parameters)
  : TK(parameters),
    _lumping(TK::template getParam<bool>("lumping")),
    _coeff(TK::template getParam<Real>("coeff"))
{
}

template <typename TK>
Real
TimeDerivativeTempl<TK>::computeQpResidual()
{
  return _coeff * TK::_test[TK::_i][TK::_qp] * TK::_u_dot[TK::_qp];
}

template <typename TK>
Real
TimeDerivativeTempl<TK>::computeQpJacobian()
{
  return _coeff * TK::_test[TK::_i][TK::_qp] * TK::_phi[TK::_j][TK::_qp] * TK::_du_dot_du[TK::_qp];
}

template <typename TK>
void
TimeDerivativeTempl<TK>::computeJacobian()
{
  if (_lumping)
  {
    TK::prepareMatrixTag(TK::_assembly, TK::_var.number(), TK::_var.number());

    TK::precalculateJacobian();
    for (TK::_i = 0; TK::_i < TK::_test.size(); TK::_i++)
      for (TK::_j = 0; TK::_j < TK::_phi.size(); TK::_j++)
        for (TK::_qp = 0; TK::_qp < TK::_qrule->n_points(); TK::_qp++)
          TK::_local_ke(TK::_i, TK::_i) +=
              TK::_JxW[TK::_qp] * TK::_coord[TK::_qp] * computeQpJacobian();

    TK::accumulateTaggedLocalMatrix();
  }
  else
    TK::computeJacobian();
}

template class TimeDerivativeTempl<TimeKernel>;
template class TimeDerivativeTempl<VectorTimeKernel>;
