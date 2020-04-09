//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InclusionProperties.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", InclusionProperties);

InputParameters
InclusionProperties::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<Real>("a", "Ellipse semiaxis");
  params.addRequiredParam<Real>("b", "Ellipse semiaxis");
  params.addRequiredParam<Real>("lambda", "Lame's first parameter");
  params.addRequiredParam<Real>("mu", "Shear modulus (aka Lame's second parameter)");
  params.addRequiredParam<std::vector<Real>>("misfit_strains",
                                             "Vector of misfit strains in order eps_11, eps_22");
  params.addRequiredParam<MaterialPropertyName>(
      "stress_name", "Name of the material property where analytical stresses will be stored");
  params.addRequiredParam<MaterialPropertyName>(
      "strain_name", "Name of the material property where analytical total strains will be stored");
  params.addRequiredParam<MaterialPropertyName>(
      "energy_name",
      "Name of the material property where analytical elastic energies will be stored");

  return params;
}

InclusionProperties::InclusionProperties(const InputParameters & parameters)
  : Material(parameters),
    _a(getParam<Real>("a")),
    _b(getParam<Real>("b")),
    _lambda(getParam<Real>("lambda")),
    _mu(getParam<Real>("mu")),
    _misfit(getParam<std::vector<Real>>("misfit_strains")),
    _stress(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("stress_name"))),
    _strain(declareProperty<RankTwoTensor>(getParam<MaterialPropertyName>("strain_name"))),
    _elastic_energy(declareProperty<Real>(getParam<MaterialPropertyName>("energy_name")))
{
  if (_misfit.size() != 2)
    mooseError("Supply 2 misfit_strains in order eps_11, eps_22 in InclusionProperties.");

  _nu = _lambda / 2.0 / (_lambda + _mu);
  _kappa = 3 - 4 * _nu;
  precomputeInteriorProperties();
}

void
InclusionProperties::computeQpProperties()
{
  Real x = _q_point[_qp](0);
  Real y = _q_point[_qp](1);
  if (x * x / _a / _a + y * y / _b / _b < 1)
  {
    // Inside the inclusion
    _stress[_qp] = _stress_int;
    _strain[_qp] = _total_strain_int;
    _elastic_energy[_qp] = _elastic_energy_int;
  }
  else
  {
    // Outside the inclusion
    Real l = 0.5 * (x * x + y * y - _a * _a - _b * _b // Parameter l called lambda in the paper
                    + std::sqrt(Utility::pow<2>((x * x + y * y - _a * _a + _b * _b)) +
                                4 * (_a * _a - _b * _b) * y * y));
    Real rho_a = _a / sqrt(_a * _a + l);
    Real rho_b = _b / sqrt(_b * _b + l);
    Real m_x = x / (_a * _a + l);
    Real m_y = y / (_b * _b + l);
    Real n_x = m_x / std::sqrt(m_x * m_x + m_y * m_y);
    Real n_y = m_y / std::sqrt(m_x * m_x + m_y * m_y);
    Real T_6 = rho_a * rho_a + rho_b * rho_b - 4 * rho_a * rho_a * n_x * n_x -
               4 * rho_b * rho_b * n_y * n_y - 4;

    Real H11 = rho_a * _b *
                   (_a * rho_b + _b * rho_a + 2 * _a * rho_a * rho_a * rho_b +
                    _b * Utility::pow<3>(rho_a)) /
                   Utility::pow<2>((_a * rho_b + _b * rho_a)) +
               n_x * n_x * (2 - 6 * rho_a * rho_a + (8 * rho_a * rho_a + T_6) * n_x * n_x);

    Real H22 = rho_b * _a *
                   (_a * rho_b + _b * rho_a + 2 * _b * rho_a * rho_b * rho_b +
                    _a * Utility::pow<3>(rho_b)) /
                   Utility::pow<2>((_a * rho_b + _b * rho_a)) +
               n_y * n_y * (2 - 6 * rho_b * rho_b + (8 * rho_b * rho_b + T_6) * n_y * n_y);

    Real H12 = (_a * _a * rho_a * rho_a * rho_b * rho_b + _b * _b * rho_a * rho_a +
                _a * _b * rho_a * rho_b) /
                   Utility::pow<2>((_a * rho_b + _b * rho_a)) -
               rho_b * rho_b * n_x * n_x - rho_a * rho_a * n_y * n_y +
               (4 * rho_a * rho_a + 4 * rho_b * rho_b + T_6) * n_x * n_x * n_y * n_y;

    Real H31 = 2 * (_b * rho_a / (_a * rho_b + _b * rho_a) - n_x * n_x);
    Real H32 = 2 * (_a * rho_b / (_a * rho_b + _b * rho_a) - n_y * n_y);

    Real H41 = n_x * n_y *
               (1 - 3 * rho_a * rho_a + (6 * rho_a * rho_a + 2 * rho_b * rho_b + T_6) * n_x * n_x);
    Real H42 = n_x * n_y *
               (1 - 3 * rho_b * rho_b + (2 * rho_a * rho_a + 6 * rho_b * rho_b + T_6) * n_y * n_y);

    _stress[_qp](0, 0) =
        4 * _mu * rho_a * rho_b / (_kappa + 1) * (H11 * _misfit[0] + H12 * _misfit[1]);
    _stress[_qp](1, 1) =
        4 * _mu * rho_a * rho_b / (_kappa + 1) * (H12 * _misfit[0] + H22 * _misfit[1]);
    _stress[_qp](2, 2) =
        4 * _mu * rho_a * rho_b / (_kappa + 1) * _nu * (H31 * _misfit[0] + H32 * _misfit[1]);
    _stress[_qp](0, 1) = _stress[_qp](1, 0) =
        4 * _mu * rho_a * rho_b / (_kappa + 1) * (H41 * _misfit[0] + H42 * _misfit[1]);

    Real J1 = rho_a * rho_a * rho_b * _b / (_a * rho_b + _b * rho_a);
    Real J11 = Utility::pow<4>(rho_a) * rho_b * _b / (3 * _a * _a) * (2 * _a * rho_b + _b * rho_a) /
               Utility::pow<2>((_a * rho_b + _b * rho_a));
    Real J12 = Utility::pow<3>(rho_a) * Utility::pow<3>(rho_b) /
               Utility::pow<2>((_a * rho_b + _b * rho_a));
    Real J2 = rho_b * rho_b * rho_a * _a / (_a * rho_b + _b * rho_a);
    Real J22 = Utility::pow<4>(rho_b) * rho_a * _a / (3 * _b * _b) * (2 * _b * rho_a + _a * rho_b) /
               Utility::pow<2>((_a * rho_b + _b * rho_a));

    Real G1111 = ((1 - 2 * _nu) * J1 + 3 * _a * _a * J11) / (2 * (1 - _nu)) +
                 rho_a * rho_b * n_x * n_x / (2 * (1 - _nu)) *
                     (2 + 2 * _nu - 6 * rho_a * rho_a + (8 * rho_a * rho_a + T_6) * n_x * n_x);
    Real G1122 = ((2 * _nu - 1) * J1 + _b * _b * J12) / (2 * (1 - _nu)) +
                 rho_a * rho_b / (2 * (1 - _nu)) *
                     ((1 - rho_a * rho_a) * n_y * n_y + (1 - 2 * _nu - rho_b * rho_b) * n_x * n_x +
                      (4 * rho_a * rho_a + 4 * rho_b * rho_b + T_6) * n_x * n_x * n_y * n_y);
    Real G2211 = ((2 * _nu - 1) * J2 + _a * _a * J12) / (2 * (1 - _nu)) +
                 rho_a * rho_b / (2 * (1 - _nu)) *
                     ((1 - rho_b * rho_b) * n_x * n_x + (1 - 2 * _nu - rho_a * rho_a) * n_y * n_y +
                      (4 * rho_a * rho_a + 4 * rho_b * rho_b + T_6) * n_x * n_x * n_y * n_y);
    Real G2222 = ((1 - 2 * _nu) * J2 + 3 * _b * _b * J22) / (2 * (1 - _nu)) +
                 rho_a * rho_b * n_y * n_y / (2 * (1 - _nu)) *
                     (2 + 2 * _nu - 6 * rho_b * rho_b + (8 * rho_a * rho_a + T_6) * n_y * n_y);
    Real G1211 =
        rho_a * rho_b * n_x * n_y / (2 * (1 - _nu)) *
        (1 - 3 * rho_a * rho_a + (6 * rho_a * rho_a + 2 * rho_b * rho_b + T_6) * n_x * n_x);
    Real G1222 =
        rho_a * rho_b * n_x * n_y / (2 * (1 - _nu)) *
        (1 - 3 * rho_b * rho_b + (2 * rho_a * rho_a + 6 * rho_b * rho_b + T_6) * n_y * n_y);

    // Outside the inclusion, total strain = elastic strain so we only need to
    // calculate one strain tensor
    _strain[_qp](0, 0) = G1111 * _misfit[0] + G1122 * _misfit[1];
    _strain[_qp](1, 1) = G2211 * _misfit[0] + G2222 * _misfit[1];
    _strain[_qp](0, 1) = _strain[_qp](1, 0) = G1211 * _misfit[0] + G1222 * _misfit[1];

    _elastic_energy[_qp] = 0.5 * _stress[_qp].doubleContraction(_strain[_qp]);
  }
}

void
InclusionProperties::precomputeInteriorProperties()
{
  Real t = _b / _a;
  Real T11 = 1 + 2 / t;
  Real T12 = 1;
  Real T21 = 1;
  Real T22 = 1 + 2 * t;
  Real T31 = (3 - _kappa) / 2 * (1 + 1 / t);
  Real T32 = (3 - _kappa) / 2 * (1 + t);

  _stress_int(0, 0) =
      -4 * _mu * t / (_kappa + 1) / (1 + t) / (1 + t) * (T11 * _misfit[0] + T12 * _misfit[1]);
  _stress_int(1, 1) =
      -4 * _mu * t / (_kappa + 1) / (1 + t) / (1 + t) * (T21 * _misfit[0] + T22 * _misfit[1]);
  _stress_int(2, 2) =
      -4 * _mu * t / (_kappa + 1) / (1 + t) / (1 + t) * (T31 * _misfit[0] + T32 * _misfit[1]);

  Real S11 = t * (t + 3 + _kappa * (1 + t));
  Real S12 = t * (1 + 3 * t - _kappa * (1 + t));
  Real S21 = t + 3 - _kappa * (1 + t);
  Real S22 = 1 + 3 * t + _kappa * (1 + t);

  _total_strain_int(0, 0) =
      1 / (_kappa + 1) / (1 + t) / (1 + t) * (S11 * _misfit[0] + S12 * _misfit[1]);
  _total_strain_int(1, 1) =
      1 / (_kappa + 1) / (1 + t) / (1 + t) * (S21 * _misfit[0] + S22 * _misfit[1]);
  // Inside the inclusion, elastic strain = total strain - eigenstrain
  _elastic_strain_int(0, 0) = _total_strain_int(0, 0) - _misfit[0];
  _elastic_strain_int(1, 1) = _total_strain_int(1, 1) - _misfit[1];

  _elastic_energy_int = 0.5 * _stress_int.doubleContraction(_elastic_strain_int);
}
