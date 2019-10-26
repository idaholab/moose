//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZM3DCLaw.h"

registerMooseObject("TensorMechanicsApp", CZM3DCLaw);

template <>
InputParameters
validParams<CZM3DCLaw>()
{
  InputParameters params = validParams<CZMMaterialBase>();
  params.addClassDescription("3DC cohseive law model, no damage");
  params.addRequiredParam<Real>(
      "normal_gap_at_maximum_normal_traction",
      "the value of normal gap at which maximum normal traction is achieved");
  params.addRequiredParam<Real>(
      "tangential_gap_at_maximum_shear_traction",
      "the value of tangential gap at which maximum shear traction is achieved");
  params.addRequiredParam<Real>("maximum_normal_traction",
                                "The maximum normal traction the interface can sustain");
  params.addRequiredParam<Real>("maximum_shear_traction",
                                "The maximum shear traction the interface can sustain");
  params.addClassDescription("Simple Exponential cohseive law model, with damage");
  return params;
}

CZM3DCLaw::CZM3DCLaw(const InputParameters & parameters)
  : CZMMaterialBase(parameters), _deltaU0(3, 0), _maxAllowableTraction(3, 0)
{

  const_cast<std::vector<Real> &>(_deltaU0)[0] =
      getParam<Real>("normal_gap_at_maximum_normal_traction");
  const_cast<std::vector<Real> &>(_maxAllowableTraction)[0] =
      getParam<Real>("maximum_normal_traction");

  for (unsigned int i = 1; i < 3; i++)
  {
    const_cast<std::vector<Real> &>(_deltaU0)[i] =
        getParam<Real>("tangential_gap_at_maximum_shear_traction") * std::sqrt(2);
    const_cast<std::vector<Real> &>(_maxAllowableTraction)[i] =
        getParam<Real>("maximum_shear_traction");
  }
  // const_cast<std::vector<Real> &>(_deltaU0).push_back(_deltaU0[1]);
  // const_cast<std::vector<Real> &>(_maxAllowableTraction).push_back(_maxAllowableTraction[1]);
}

RealVectorValue
CZM3DCLaw::computeTraction()
{
  RealVectorValue traction_local;

  // just a temporaty container for auxiliary calcualtions
  Real aa;

  // convention N, T, S
  Real X, expX, A_i, B_i;

  X = 0;
  for (unsigned int i = 0; i < 3; i++)
  {
    aa = _displacement_jump[_qp](i) / _deltaU0[i];
    if (i > 0)
      aa *= aa; // square for shear component

    X += aa;
  }

  expX = std::exp(-X);

  for (unsigned int i = 0; i < 3; i++)
  {

    if (i == 0)
      aa = std::exp(1);
    else
      aa = std::sqrt(2 * std::exp(1));

    A_i = _maxAllowableTraction[i] * aa;
    B_i = _displacement_jump[_qp](i) / _deltaU0[i];

    traction_local(i) = A_i * B_i * expX;
  }

  return traction_local;
}

RankTwoTensor
CZM3DCLaw::computeTractionDerivatives()
{
  RankTwoTensor traction_spatial_derivatives_local;

  // this function compute partial derivates of Tn[0][:], Tt[1][:], Ts[2][:]
  // w.r.t. dun, dut, dus
  // T_i = A_i*B_i*exp(-X) with:
  // A_i = \sigma_i,max * (\alpha_i*e)^{1/\alpha_i} with \alpha_i = 1 for i==n
  // \alpha_i = 2 for i!=n
  // B_i = \delta_u,i / \delta_0,i
  // X = sum_i=1^3{(\delta_u,i / \delta_0,i)^\alpha_i}  with \alpha_i = 1 for i==n
  // \alpha_i = 2 for i!=n

  // dTi_duj = A_i * ( dBi_duj * exp(-X) + B_i * exp(-X) * dX_duj  )
  //         = A_i * ( exp(-X) * (dBi_duj + B_i * dX_duj ) )

  // just a temporaty container for auxiliary calcualtions
  Real aa;

  // convention N, T, S
  unsigned int i, j;
  Real expX, X;

  // compute X and the exponential term
  aa = 0;
  X = 0;
  for (i = 0; i < 3; i++)
  {
    aa = _displacement_jump[_qp](i) / _deltaU0[i];
    if (i > 0)
      aa *= aa;
    X += aa;
  }
  expX = std::exp(-X);

  // compute partial derivatives in local coordaintes w.r.t. the master surface siplacement
  //            | dTn/dun dTn/dut dTn/dus |
  // dTi_duj  = | dTt/dun dTt/dut dTt/dus | = _TractionDerivativeLocal[i][j]
  //            | dTs/dun dTs/dut dTs/dus |
  Real A_i, B_i;
  Real dBi_dui, dX_duj;

  for (i = 0; i < 3; i++)
  {

    // compute A_i
    if (i == 0) // alpha = 1
      A_i = std::exp(1);
    else // alpha = 2
      A_i = std::sqrt(2 * std::exp(1));

    A_i *= _maxAllowableTraction[i];

    // compute B_i
    B_i = _displacement_jump[_qp](i) / _deltaU0[i];

    for (j = 0; j < 3; j++)
    {

      // add term for diagonal entry dBi_dui
      dBi_dui = 0;
      if (i == j)
        dBi_dui = 1 / _deltaU0[j];

      // compute the derivative of the argument of exponential
      if (j == 0) // alpha = 1
        dX_duj = 1. / _deltaU0[j];
      else // alpha = 2
        dX_duj = 2. * _displacement_jump[_qp](j) / (_deltaU0[j] * _deltaU0[j]);

      traction_spatial_derivatives_local(i, j) =
          A_i * expX * (dBi_dui - B_i * dX_duj); // the minus sign is due to exp(-X)
    }
  }

  return traction_spatial_derivatives_local;
}
