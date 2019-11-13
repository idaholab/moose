//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SalehaniIrani3DCTraction.h"

registerMooseObject("TensorMechanicsApp", SalehaniIrani3DCTraction);

template <>
InputParameters
validParams<SalehaniIrani3DCTraction>()
{
  InputParameters params = validParams<CZMMaterialBase>();
  params.addClassDescription("3D Coupled (3DC) cohesive law of Salehani and Irani with no damage");
  params.addRequiredParam<Real>(
      "normal_gap_at_maximum_normal_traction",
      "The value of normal gap at which maximum normal traction is achieved");
  params.addRequiredParam<Real>(
      "tangential_gap_at_maximum_shear_traction",
      "The value of tangential gap at which maximum shear traction is achieved");
  params.addRequiredParam<Real>("maximum_normal_traction",
                                "The maximum normal traction the interface can sustain");
  params.addRequiredParam<Real>("maximum_shear_traction",
                                "The maximum shear traction the interface can sustain");
  return params;
}

SalehaniIrani3DCTraction::SalehaniIrani3DCTraction(const InputParameters & parameters)
  : CZMMaterialBase(parameters),
    _delta_u0({getParam<Real>("normal_gap_at_maximum_normal_traction"),
               std::sqrt(2) * getParam<Real>("tangential_gap_at_maximum_shear_traction"),
               std::sqrt(2) * getParam<Real>("tangential_gap_at_maximum_shear_traction")}),
    _max_allowable_traction({getParam<Real>("maximum_normal_traction"),
                             getParam<Real>("maximum_shear_traction"),
                             getParam<Real>("maximum_shear_traction")})
{
}

RealVectorValue
SalehaniIrani3DCTraction::computeTraction()
{

  // The convention for ordering the traction is N, T, S, where N is the normal direction, and T and
  // S are two arbitrary tangential directions.
  RealVectorValue traction_local;

  // temporary containers for auxiliary calculations
  Real aa, x, exp_x, a_i, b_i;
  // index variable to avoid multiple redefinitions
  unsigned int i;

  x = 0;
  for (i = 0; i < 3; i++)
  {
    aa = _displacement_jump[_qp](i) / _delta_u0[i];
    if (i > 0)
      aa *= aa; // square for shear component

    x += aa;
  }

  exp_x = std::exp(-x);

  for (i = 0; i < 3; i++)
  {
    if (i == 0)
      aa = std::exp(1);
    else
      aa = std::sqrt(2 * std::exp(1));

    a_i = _max_allowable_traction[i] * aa;
    b_i = _displacement_jump[_qp](i) / _delta_u0[i];
    traction_local(i) = a_i * b_i * exp_x;
  }

  return traction_local;
}

RankTwoTensor
SalehaniIrani3DCTraction::computeTractionDerivatives()
{
  RankTwoTensor traction_jump_derivatives_local;

  // this function computes the partial derivates of Tn[0][:], Tt[1][:], Ts[2][:]
  // wrt dun, dut, dus
  // T_i = a_i*b_i*exp(-x) with:
  // a_i = \sigma_i,max * (\alpha_i*e)^{1/\alpha_i} with \alpha_i = 1 for i==n
  // \alpha_i = 2 for i!=n
  // b_i = \delta_u,i / \delta_0,i
  // x = sum_i=1^3{(\delta_u,i / \delta_0,i)^\alpha_i}  with \alpha_i = 1 for i==n
  // \alpha_i = 2 for i!=n

  // dTi_duj = a_i * ( dBi_duj * exp(-x) + b_i * exp(-x) * dX_duj  )
  //         = a_i * ( exp(-x) * (dBi_duj + b_i * dX_duj ) )

  // temporary containers for auxiliary calculations
  Real aa, exp_x, x;
  // index variables to avoid multiple redefinitions
  unsigned int i, j;

  // compute x and the exponential term
  aa = 0;
  x = 0;
  for (i = 0; i < 3; i++)
  {
    aa = _displacement_jump[_qp](i) / _delta_u0[i];
    if (i > 0)
      aa *= aa;
    x += aa;
  }
  exp_x = std::exp(-x);

  // compute partial derivatives in local coordiante wrt the displacement jump
  //            | dTn/dun dTn/dut dTn/dus |
  // dTi_duj  = | dTt/dun dTt/dut dTt/dus | = _traction_derivative[i][j]
  //            | dTs/dun dTs/dut dTs/dus |
  Real a_i, b_i;
  Real dBi_dui, dX_duj;

  for (i = 0; i < 3; i++)
  {
    if (i == 0) // alpha = 1
      a_i = std::exp(1);
    else // alpha = 2
      a_i = std::sqrt(2 * std::exp(1));

    a_i *= _max_allowable_traction[i];
    b_i = _displacement_jump[_qp](i) / _delta_u0[i];

    for (j = 0; j < 3; j++)
    {

      dBi_dui = 0;
      if (i == j)
        dBi_dui = 1 / _delta_u0[j];

      if (j == 0) // alpha = 1
        dX_duj = 1. / _delta_u0[j];
      else // alpha = 2
        dX_duj = 2. * _displacement_jump[_qp](j) / (_delta_u0[j] * _delta_u0[j]);

      traction_jump_derivatives_local(i, j) =
          a_i * exp_x * (dBi_dui - b_i * dX_duj); // the minus sign is due to exp(-x)
    }
  }

  return traction_jump_derivatives_local;
}
