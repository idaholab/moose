/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACGrGrPoly.h"

template <>
InputParameters
validParams<ACGrGrPoly>()
{
  InputParameters params = validParams<ACGrGrBase>();
  params.addClassDescription("Grain-Boundary model poly-crystaline interface Allen-Cahn Kernel");
  params.addParam<unsigned int>(
      "second_phase_op_num", 0,
      "specifies the total number of grains/variants of the second phase");
  params.addParam<Real>(
      "en_ratio", 1.0,
      "Ratio of surface energy to GB energy, e.g., interphase energy");
  params.addParam<Real>("second_phase_en_ratio", 1.0,
                        "Ratio of second-phase to parent-phase GB energy");
  params.addParam<Real>("mob_ratio", 1.0,
                        "Ratio of surface energy to GB mobility");
  return params;
}

ACGrGrPoly::ACGrGrPoly(const InputParameters &parameters)
    : ACGrGrBase(parameters), _gamma(getMaterialProperty<Real>("gamma_asymm")),
      _op_index(getParam<unsigned int>("op_index")),
      _second_phase_op_num(getParam<unsigned int>("second_phase_op_num")),
      _en_ratio(getParam<Real>("en_ratio")),
      _second_phase_en_ratio(getParam<Real>("second_phase_en_ratio")),
      _mob_ratio(getParam<Real>("mob_ratio"))

{}

Real
ACGrGrPoly::computeDFDOP(PFFunctionType type)
{
  // Sum all other order parameters
  Real SumEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumEtaj += (*_vals[i])[_qp] * (*_vals[i])[_qp];

  // Sum all other order parameters of either phase for the two-phase case
  Real SumEtak = 0.0;
  if (_second_phase_op_num > 0) {
    // second-phase
    if (_op_index > _op_num - _second_phase_op_num) {
      for (unsigned int i = _op_num - _second_phase_op_num; i < _op_num; ++i)
        SumEtak += (*_vals[i])[_qp] * (*_vals[i])[_qp];
    }
    // parent-phase
    else {
      for (unsigned int i = 0; i < _op_num - _second_phase_op_num; ++i)
        SumEtak += (*_vals[i])[_qp] * (*_vals[i])[_qp];
    }
  }

  // Calculate either the residual or Jacobian of the two-phase or single-phase
  // grain growth free energy
  switch (type)
  {
    case Residual:
    {
      const Real tgrad_correction =
          _grad_T ? _tgrad_corr_mult[_qp] * _grad_u[_qp] * (*_grad_T)[_qp] : 0.0;
      // two-phase case
      if (_second_phase_op_num > 0) {
        // second-phase
        if (_op_index > _op_num - _second_phase_op_num)
          return _mob_ratio * _mu[_qp] *
                     (_u[_qp] * _u[_qp] * _u[_qp] - _u[_qp] +
                      2.0 * _second_phase_en_ratio * _gamma[_qp] * _u[_qp] *
                          SumEtak +
                      2.0 * _en_ratio * _gamma[_qp] * _u[_qp] *
                          (SumEtaj - SumEtak)) +
                 tgrad_correction;
        // parent-phase
        else
          return _mu[_qp] * (_u[_qp] * _u[_qp] * _u[_qp] - _u[_qp] +
                             2.0 * _gamma[_qp] * _u[_qp] * SumEtak +
                             2.0 * _en_ratio * _gamma[_qp] * _u[_qp] *
                                 (SumEtaj - SumEtak)) +
                 tgrad_correction;
      }
      // single-phase case
      return _mu[_qp] *
                 (_u[_qp] * _u[_qp] * _u[_qp] - _u[_qp] + 2.0 * _gamma[_qp] * _u[_qp] * SumEtaj) +
             tgrad_correction;
    }

    case Jacobian:
    {
      const Real tgrad_correction =
          _grad_T ? _tgrad_corr_mult[_qp] * _grad_phi[_j][_qp] * (*_grad_T)[_qp] : 0.0;
      // two-phase case
      if (_second_phase_op_num > 0) {
        // second-phase
        if (_op_index > _op_num - _second_phase_op_num)
          return _mob_ratio * _mu[_qp] *
                     (_phi[_j][_qp] *
                      (3.0 * _u[_qp] * _u[_qp] - 1.0 +
                       2.0 * _second_phase_en_ratio * _gamma[_qp] * SumEtak +
                       2.0 * _en_ratio * _gamma[_qp] * (SumEtaj - SumEtak))) +
                 tgrad_correction;
        // parent-phase
        else
          return _mu[_qp] * (_phi[_j][_qp] * (3.0 * _u[_qp] * _u[_qp] - 1.0 +
                                              2.0 * _gamma[_qp] * SumEtak +
                                              2.0 * _en_ratio * _gamma[_qp] *
                                                  (SumEtaj - SumEtak))) +
                 tgrad_correction;
      }
      // single-phase case
      return _mu[_qp] *
                 (_phi[_j][_qp] * (3.0 * _u[_qp] * _u[_qp] - 1.0 + 2.0 * _gamma[_qp] * SumEtaj)) +
             tgrad_correction;
    }

    default:
      mooseError("Invalid type passed in");
  }
}

Real
ACGrGrPoly::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _op_num; ++i)
    if (jvar == _vals_var[i])
    {
      // two-phase case
      if (_second_phase_op_num > 0) {
        // second-phase
        if (_op_index > _op_num - _second_phase_op_num) {
          if (jvar >= _op_num - _second_phase_op_num)
            return _mob_ratio * _L[_qp] * _test[_i][_qp] * _mu[_qp] * 4.0 *
                   _second_phase_en_ratio * _gamma[_qp] * _u[_qp] *
                   (*_vals[i])[_qp] * _phi[_j][_qp];
          else
            return _mob_ratio * _L[_qp] * _test[_i][_qp] * _mu[_qp] * 4.0 *
                   _en_ratio * _gamma[_qp] * _u[_qp] * (*_vals[i])[_qp] *
                   _phi[_j][_qp];
        }
        // parent-phase
        else {
          if (jvar >= _op_num - _second_phase_op_num)
            return _L[_qp] * _test[_i][_qp] * _mu[_qp] * 4.0 * _gamma[_qp] *
                   _u[_qp] * (*_vals[i])[_qp] * _phi[_j][_qp];
          else
            return _L[_qp] * _test[_i][_qp] * _mu[_qp] * 4.0 * _en_ratio *
                   _gamma[_qp] * _u[_qp] * (*_vals[i])[_qp] * _phi[_j][_qp];
        }
      }
      // single-phase case
      // Derivative of SumEtaj
      const Real dSumEtaj = 2.0 * (*_vals[i])[_qp] * _phi[_j][_qp];
      const Real dDFDOP = _mu[_qp] * 2.0 * _gamma[_qp] * _u[_qp] * dSumEtaj;

      return _L[_qp] * _test[_i][_qp] * dDFDOP;
    }

  return 0.0;
}
