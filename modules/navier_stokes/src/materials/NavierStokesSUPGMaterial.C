//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesSUPGMaterial.h"
#include "SinglePhaseFluidProperties.h"
#include "IdealGasFluidProperties.h"
#include "NavierStokesMethods.h"
#include "NS.h"
#include "InputErrorChecking.h"

// MOOSE includes
#include "FEProblem.h"
#include "Assembly.h"
#include "SubProblem.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/dense_matrix_base_impl.h"
#include "libmesh/dense_matrix_impl.h"
#include "libmesh/utility.h"

#include "metaphysicl/raw_type.h"

// C++ includes
#include <iomanip>

registerADMooseObject("NavierStokesApp", NavierStokesSUPGMaterial);

namespace nms = NS;

defineADValidParams(
    NavierStokesSUPGMaterial, ADMaterial, params.addRequiredCoupledVar(nms::porosity, "porosity");
    params.addCoupledVar(nms::T_solid, "T_solid");

    params.addParam<RealVectorValue>(nms::acceleration,
                                     NS_DEFAULT_VALUES::acceleration,
                                     "The gravitational acceleration components");

    params.addCoupledVar(nms::heat_source, "The coupled heat source");
    params.addParam<Real>("heat_source_scaling_factor",
                          1.0,
                          "Scaling factor applied to the heat source");

    params.addParam<UserObjectName>(nms::fluid, "Fluid userobject");

    params.addParam<bool>("mass_equation",
                          true,
                          "Whether to include the conservation of mass equation");
    params.addParam<bool>("momentum_equations",
                          true,
                          "Whether to include the conservation of momentum equation(s)");
    params.addParam<bool>("fluid_energy_equation",
                          true,
                          "Whether to include the conservation of fluid energy equation");
    params.addParam<bool>("viscous_stress",
                          false,
                          "Whether to include the viscous stress term in the momentum equation(s)");
    params.addClassDescription("Material for computing matrices and vectors associated with "
                               "the coupled SUPG stabilization scheme."););

NavierStokesSUPGMaterial::NavierStokesSUPGMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<ADMaterial>(parameters),
    _mesh_dim(_mesh.dimension()),
    _N(_mesh_dim + 2),
    _rz_coord(_subproblem.getAxisymmetricRadialCoord()),

    _mass_eqn(getParam<bool>("mass_equation")),
    _momentum_eqn(getParam<bool>("momentum_equations")),
    _fluid_energy_eqn(getParam<bool>("fluid_energy_equation")),
    _viscous_stress(getParam<bool>("viscous_stress") && _momentum_eqn),

    // nonlinear variables
    _rho(getADMaterialProperty<Real>(nms::density)),
    _momentum(getADMaterialProperty<RealVectorValue>(nms::momentum)),
    _rho_et(getADMaterialProperty<Real>(nms::total_energy_density)),

    // nonlinear variables - space derivatives
    _grad_rho(getADMaterialProperty<RealVectorValue>(nms::grad(nms::density))),
    _grad_rho_u(getADMaterialProperty<RealVectorValue>(nms::grad(nms::momentum_x))),
    _grad_rho_v(getADMaterialProperty<RealVectorValue>(nms::grad(nms::momentum_y))),
    _grad_rho_w(getADMaterialProperty<RealVectorValue>(nms::grad(nms::momentum_z))),
    _grad_rho_et(getADMaterialProperty<RealVectorValue>(nms::grad(nms::total_energy_density))),

    // nonlinear variables - time derivatives
    _drho_dt(getADMaterialProperty<Real>(nms::time_deriv(nms::density))),
    _drhou_dt(getADMaterialProperty<Real>(nms::time_deriv(nms::momentum_x))),
    _drhov_dt(getADMaterialProperty<Real>(nms::time_deriv(nms::momentum_y))),
    _drhow_dt(getADMaterialProperty<Real>(nms::time_deriv(nms::momentum_z))),
    _drhoEt_dt(getADMaterialProperty<Real>(nms::time_deriv(nms::total_energy_density))),

    // auxiliary variables
    _velocity(getADMaterialProperty<RealVectorValue>(nms::velocity)),
    _grad_vel_x(getADMaterialProperty<RealVectorValue>(nms::grad(nms::velocity_x))),
    _grad_vel_y(getADMaterialProperty<RealVectorValue>(nms::grad(nms::velocity_y))),
    _grad_vel_z(getADMaterialProperty<RealVectorValue>(nms::grad(nms::velocity_z))),
    _grad_grad_vel_x(getADMaterialProperty<RealTensorValue>(nms::grad(nms::grad(nms::velocity_x)))),
    _grad_grad_vel_y(getADMaterialProperty<RealTensorValue>(nms::grad(nms::grad(nms::velocity_y)))),
    _grad_grad_vel_z(getADMaterialProperty<RealTensorValue>(nms::grad(nms::grad(nms::velocity_z)))),
    _eps(coupledValue(nms::porosity)),
    _grad_eps(coupledGradient(nms::porosity)),
    _p(getADMaterialProperty<Real>(nms::pressure)),
    _specific_total_enthalpy(getADMaterialProperty<Real>(nms::specific_total_enthalpy)),
    _specific_total_energy(getADMaterialProperty<Real>(nms::specific_internal_energy)),
    _v(getADMaterialProperty<Real>(nms::v)),
    _speed(getADMaterialProperty<Real>(nms::speed)),

    _fluid(_fluid_energy_eqn ? &getUserObject<SinglePhaseFluidProperties>(nms::fluid) : nullptr),
    _fluid_ideal_gas(nullptr),

    _acceleration(getParam<RealVectorValue>(nms::acceleration)),
    _kappa(_fluid_energy_eqn ? &getADMaterialProperty<Real>(nms::kappa) : nullptr),
    _dkappa_dp(_fluid_energy_eqn ? &getMaterialPropertyDerivative<Real>(nms::kappa, nms::pressure)
                                 : nullptr),
    _dkappa_dT(_fluid_energy_eqn ? &getMaterialPropertyDerivative<Real>(nms::kappa, nms::T_fluid)
                                 : nullptr),
    _mu_eff(_viscous_stress ? &getADMaterialProperty<Real>(nms::mu_eff) : nullptr),
    _dmu_eff_dp(_viscous_stress ? &getMaterialPropertyDerivative<Real>(nms::mu_eff, nms::pressure)
                                : nullptr),
    _dmu_eff_dT(_viscous_stress ? &getMaterialPropertyDerivative<Real>(nms::mu_eff, nms::T_fluid)
                                : nullptr),

    _convective_heat_transfer(_fluid_energy_eqn && isCoupled(nms::T_solid)),
    _alpha(_convective_heat_transfer ? &getADMaterialProperty<Real>(nms::alpha) : nullptr),
    _T_solid(_convective_heat_transfer ? adCoupledValue(nms::T_solid) : _ad_zero),
    _cL(_momentum_eqn ? &getADMaterialProperty<RealVectorValue>(nms::cL) : nullptr),
    _cQ(_momentum_eqn ? &getADMaterialProperty<RealVectorValue>(nms::cQ) : nullptr),
    _T_fluid(getADMaterialProperty<Real>(nms::T_fluid)),
    _grad_T_fluid(getADMaterialProperty<RealVectorValue>(nms::grad(nms::T_fluid))),
    _hess_T_fluid(getADMaterialProperty<RealTensorValue>(nms::grad(nms::grad(nms::T_fluid)))),
    _grad_pressure(getADMaterialProperty<RealVectorValue>(nms::grad(nms::pressure))),
    _heat_source(isCoupled(nms::heat_source) ? coupledValue(nms::heat_source) : _zero),
    _scaling_factor(getParam<Real>("heat_source_scaling_factor")),

    // materials created by this material
    _A(declareADProperty<std::vector<DenseMatrix<Real>>>(nms::A)),
    _R(declareADProperty<DenseVector<Real>>(nms::R)),
    _S(declareADProperty<DenseVector<Real>>(nms::S)),
    _F(declareADProperty<std::vector<DenseVector<Real>>>(nms::F)),
    _G(declareADProperty<std::vector<DenseVector<Real>>>(nms::G)),
    _dU_dt(declareADProperty<DenseVector<Real>>(nms::dUdt)),
    // To be set in the constructor body below
    _single_phase_dP_and_dT(nullptr),
    // Initialize class helper variables
    _vel2(0.),
    _total_energy(0.)
{
  // dynamic cast to determine how specialized the provided FluidProperties object is.
  if (_fluid != nullptr)
  {
    _fluid_ideal_gas = dynamic_cast<const IdealGasFluidProperties *>(_fluid);
    if (_fluid_ideal_gas)
      _single_phase_dP_and_dT = &NavierStokesSUPGMaterial::computeIdealGasdPdT;
    else
      _single_phase_dP_and_dT = &NavierStokesSUPGMaterial::computeSinglePhasedPdT;
  }

  // need to specify at least one equation
  if (!_mass_eqn && !_momentum_eqn && !_fluid_energy_eqn)
    errorMessage(parameters,
                 "At least one of the mass, momentum, and fluid energy "
                 "equations must be included!");
}

void
NavierStokesSUPGMaterial::computeIdealGasdPdT()
{
  // When we know we're dealing with an ideal gas EOS, we can use
  // simplified expressions for _dP and _dT.
  ADReal gamma = _fluid_ideal_gas->gamma();
  ADReal cv = _fluid->cv_from_v_e(_v[_qp], _specific_total_energy[_qp]);
  ADReal coeff = 1. / (_rho[_qp] * cv);
  _dT.resize(_N);
  _dP.resize(_N);

  // derivative of pressure with respect to density
  _dP[0] = 0.5 * (gamma - 1) * _vel2;

  // derivative of pressure with respect to momentum
  for (unsigned int i = 1; i < _mesh_dim + 1; ++i)
    _dP[i] = (1 - gamma) * _velocity[_qp](i - 1);

  // derivative of pressure with respect to energy
  _dP[_N - 1] = gamma - 1;

  // derivative of temperature with respect to density
  _dT[0] = coeff * (_vel2 - _total_energy);

  // derivative of temperature with respect to momentum
  for (unsigned int i = 1; i < _mesh_dim + 1; ++i)
    _dT[i] = coeff * -_velocity[_qp](i - 1);

  // derivative of temperature with respect to energy
  _dT[_N - 1] = coeff;
}

void
NavierStokesSUPGMaterial::computeSinglePhasedPdT()
{
  // To allow this class to work with non ideal gas equations of
  // state, we can express derivatives of the pressure and temperature
  // with respect to the conserved variables as:
  // dp/dU_i = dp/dv * dv/dU_i + dp/de * de/dU_i
  // dT/dU_i = dT/dv * dv/dU_i + dT/de * de/dU_i
  // where pressure, p(v,e), and temperature, T(v,e), are functions of
  // specific volume (v) and internal energy (e). The derivatives dp/dv,
  // dp/de, dT/dv, dT/de and are provided by the FluidProperties
  // model, and the application provides dv/dU_i and de/dU_i.
  std::vector<ADReal> dvdU, dedU;
  dvdU.resize(_N);
  dedU.resize(_N);
  _dT.resize(_N);
  _dP.resize(_N);

  // partial derivative of specific volume and internal energy w.r.t. density
  dvdU[0] = -1.0 / _rho[_qp] / _rho[_qp];
  dedU[0] = (_vel2 - _total_energy) / _rho[_qp];

  // partial derivative of specific volume and internal energy w.r.t. momentum
  for (unsigned int i = 1; i < _mesh_dim + 1; ++i)
  {
    dvdU[i] = 0.0;
    dedU[i] = -_velocity[_qp](i - 1) / _rho[_qp];
  }

  // partial derivative of specific volume and internal energy w.r.t total energy
  dvdU[_N - 1] = 0.0;
  dedU[_N - 1] = 1.0 / _rho[_qp];

  // Get partial derivatives of p wrt (v,e) from the EOS object.
  ADReal dummy = 0, dp_dv = 0, dp_de = 0;
  _fluid->p_from_v_e(_v[_qp], _specific_total_energy[_qp], dummy, dp_dv, dp_de);

  // Get partial derivatives of T wrt (v,e) from the EOS object.
  ADReal dT_dv = 0, dT_de = 0;
  _fluid->T_from_v_e(_v[_qp], _specific_total_energy[_qp], dummy, dT_dv, dT_de);

  // Compute _dT and _dP using partial derivative formula
  for (unsigned int i = 0; i < _N; ++i)
  {
    _dT[i] = dT_dv * dvdU[i] + dT_de * dedU[i];
    _dP[i] = dp_dv * dvdU[i] + dp_de * dedU[i];
  }
}

void
NavierStokesSUPGMaterial::computeEnthalpyDerivatives()
{
  _dh.resize(_N);

  // pressure / rho partial derivative components
  for (unsigned int i = 0; i < _N; ++i)
    _dh[i] = _dP[i] / _rho[_qp];

  // total energy partial derivative components, which are only present
  // for the density and energy derivatives
  _dh[0] += -_specific_total_enthalpy[_qp] / _rho[_qp];
  _dh[_N - 1] += 1.0 / _rho[_qp];
}

void
NavierStokesSUPGMaterial::computeA()
{
  // _A[_qp][i] is the ith flux Jacobian matrix, there are _mesh_dim flux Jacobians.
  _A[_qp].resize(_N - 2);

  for (unsigned int i = 0; i < _mesh_dim; ++i)
  {
    _A[_qp][i].resize(_N, _N);
    _A[_qp][i].zero();

    ADRealVectorValue d(delta(0, i), delta(1, i), delta(2, i));

    // row corresponding to the mass equation - first and last entries are zero
    // because derivatives of momentum w.r.t. density and total energy are zero
    if (_mass_eqn)
      for (unsigned int k = 1; k < _mesh_dim + 1; ++k)
        _A[_qp][i](0, k) = d(k - 1);

    if (_fluid_energy_eqn)
    {
      // row corresponding to the fluid energy equation - each row has a
      // momentum * d(specific_total_enthalpy) contribution. The columns corresponding to the
      // momentum equation(s) also have an specific_total_enthalpy * d(momentum) contribution.
      for (unsigned int k = 0; k < _N; ++k)
        _A[_qp][i](_N - 1, k) = _rho[_qp] * _velocity[_qp](i) * _dh[k];

      for (unsigned int k = 1; k < _mesh_dim + 1; ++k)
        _A[_qp][i](_N - 1, k) += d(k - 1) * _specific_total_enthalpy[_qp];
    }

    if (_momentum_eqn)
      for (unsigned int k = 1; k < _mesh_dim + 1; ++k)
      {
        _A[_qp][i](k, k) += _velocity[_qp](i);

        // here we're not looping over columns, but rows in the momentum equation(s).
        _A[_qp][i](k, 0) += -_velocity[_qp](k - 1) * _velocity[_qp](i) + d(k - 1) * _dP[0];
        _A[_qp][i](k, _N - 1) += d(k - 1) * _dP[_N - 1];

        // momentum equation(s)
        for (unsigned int l = 1; l < _mesh_dim + 1; ++l)
          _A[_qp][i](l, k) += d(k - 1) * _velocity[_qp](l - 1) + d(l - 1) * _dP[k];
      }
  }
}

void
NavierStokesSUPGMaterial::computeF()
{
  // _F[_qp][i] is the ith inviscid flux vector, there are _mesh_dim invsicid flux vectors
  _F[_qp].resize(_N - 2);

  for (unsigned int i = 0; i < _mesh_dim; ++i)
  {
    _F[_qp][i].resize(_N);
    _F[_qp][i].zero();

    _F[_qp][i](0) = _rho[_qp] * _velocity[_qp](i);

    _F[_qp][i](_N - 1) = _rho[_qp] * _velocity[_qp](i) * _specific_total_enthalpy[_qp];

    for (unsigned int k = 1; k < _mesh_dim + 1; ++k)
      _F[_qp][i](k) =
          _rho[_qp] * _velocity[_qp](i) * _velocity[_qp](k - 1) + delta(k - 1, i) * _p[_qp];
  }
}

void
NavierStokesSUPGMaterial::computeG()
{
  // _G[_qp] is the viscous flux vector; there is 1 viscous flux vector, which is
  // all zeros except for the last entry because only the energy equation has a
  // viscous component.
  _G[_qp].resize(_N - 2);

  for (unsigned int i = 0; i < _mesh_dim; ++i)
  {
    _G[_qp][i].resize(_N);
    _G[_qp][i].zero();

    if (_fluid_energy_eqn)
      _G[_qp][i](_N - 1) = (*_kappa)[_qp] * _grad_T_fluid[_qp](i);

    if (_viscous_stress)
      for (unsigned int k = 1; k < _mesh_dim + 1; ++k)
      {
        auto grad_velocity =
            k == 1 ? _grad_vel_x[_qp] : (k == 2 ? _grad_vel_y[_qp] : _grad_vel_z[_qp]);

        _G[_qp][i](k) = (*_mu_eff)[_qp] * grad_velocity(i);
      }
  }
}

void
NavierStokesSUPGMaterial::computeS()
{
  // _S[_qp] is the vector of source terms. There is one of these vectors. No source
  // terms are present in the mass equation.
  _S[_qp].resize(_N);
  _S[_qp].zero();

  // Momentum equation source terms: - p * grad(epsilon) - gravity force + drag
  if (_momentum_eqn)
  {
    for (unsigned int d = 0; d < _N - 2; ++d)
      _S[_qp](d + 1) = -_eps[_qp] * _rho[_qp] * _acceleration(d) +
                       ((*_cL)[_qp](d) + (*_cQ)[_qp](d)) * _rho[_qp] * _velocity[_qp](d) -
                       _grad_eps[_qp](d) * _p[_qp];

    // In RZ there is an extra contribution to the source term Jacobian
    // in the radial momentum equation.
    if (_assembly.coordSystem() == Moose::COORD_RZ)
      _S[_qp](1 + _rz_coord) -= (_eps[_qp] * _p[_qp]) / _q_point[_qp](_rz_coord);
  }

  // Total energy equation source terms: - gravity power + convective heat transfer
  // - heat source in fluid
  if (_fluid_energy_eqn)
    _S[_qp](_N - 1) = -_eps[_qp] * _acceleration * _rho[_qp] * _velocity[_qp] -
                      _scaling_factor * _heat_source[_qp];

  if (_convective_heat_transfer)
    _S[_qp](_N - 1) += (*_alpha)[_qp] * (_T_fluid[_qp] - _T_solid[_qp]);
}

void
NavierStokesSUPGMaterial::computedUdt()
{
  _dU_dt[_qp].resize(_N);
  _dU_dt[_qp].zero();

  if (_mass_eqn)
    _dU_dt[_qp](0) = _drho_dt[_qp];

  if (_momentum_eqn)
  {
    ADRealVectorValue dmom_dt(_drhou_dt[_qp], _drhov_dt[_qp], _drhow_dt[_qp]);
    for (unsigned int k = 1; k < _mesh_dim + 1; ++k)
      _dU_dt[_qp](k) = dmom_dt(k - 1);
  }

  if (_fluid_energy_eqn)
    _dU_dt[_qp](_N - 1) = _drhoEt_dt[_qp];
}

void
NavierStokesSUPGMaterial::computedUdx()
{
  _dU_dx.resize(_mesh_dim);

  for (unsigned int i = 0; i < _mesh_dim; ++i)
  {
    _dU_dx[i].resize(_N);
    _dU_dx[i].zero();

    if (_mass_eqn)
      _dU_dx[i](0) = _grad_rho[_qp](i);

    if (_momentum_eqn)
    {
      ADRealVectorValue grad_mom(_grad_rho_u[_qp](i), _grad_rho_v[_qp](i), _grad_rho_w[_qp](i));

      for (unsigned int k = 1; k < _mesh_dim + 1; ++k)
        _dU_dx[i](k) = grad_mom(k - 1);
    }

    if (_fluid_energy_eqn)
      _dU_dx[i](_N - 1) = _grad_rho_et[_qp](i);
  }
}

void
NavierStokesSUPGMaterial::computeR()
{
  // _R[_qp] is the quasi-linear strong residual, computed in terms of
  // conserved variables (i.e. no change-of-variables operations appear here).
  _R[_qp].resize(_N);
  _R[_qp].zero();

  // time derivative term, porosity outside the time derivative
  _R[_qp].add(_eps[_qp], _dU_dt[_qp]);

  // inviscid flux terms, the sum of eps * A * dU_dx and F * d_eps_dx
  for (unsigned int d = 0; d < _mesh_dim; ++d)
  {
    // R += eps * A_i * dU/dx_i
    // vector_mult_add(dest, scalar_multiple, argument)
    _A[_qp][d].use_blas_lapack = false;
    _A[_qp][d].vector_mult_add(_R[_qp], _eps[_qp], _dU_dx[d]);

    // R += d(eps)/dx_i * F_i. This represents the second part of the chain rule
    // of d(eps * F)/dx, where the first part is already accounted for above.
    _R[_qp].add(_grad_eps[_qp](d), _F[_qp][d]);
  }

  // source terms
  _R[_qp] += _S[_qp];

  if (_fluid_energy_eqn)
  {
    // Add the thermal conductivity quasi-linear residual terms to the
    // last component of _R - currently we do not actually use viscous flux jacobian here.
    // Thermal conductivity can depend on both Pressure and Temperature.
    ADRealVectorValue grad_k =
        (*_dkappa_dp)[_qp] * _grad_pressure[_qp] + (*_dkappa_dT)[_qp] * _grad_T_fluid[_qp];

    _R[_qp](_N - 1) -= _eps[_qp] * (*_kappa)[_qp] * _hess_T_fluid[_qp].tr() +
                       (_eps[_qp] * grad_k + (*_kappa)[_qp] * _grad_eps[_qp]) * _grad_T_fluid[_qp];
  }

  if (_viscous_stress)
  {
    // Add the viscous stress quasi-linear residual teerms to the momentum
    // components of _R - currently we do not actually use viscous flux jacobians here.
    ADRealVectorValue grad_mu_eff =
        (*_dmu_eff_dp)[_qp] * _grad_pressure[_qp] + (*_dmu_eff_dT)[_qp] * _grad_T_fluid[_qp];

    for (unsigned int k = 1; k < _mesh_dim + 1; ++k)
    {
      auto grad_velocity =
          k == 1 ? _grad_vel_x[_qp] : (k == 2 ? _grad_vel_y[_qp] : _grad_vel_z[_qp]);
      auto grad_grad_velocity =
          k == 1 ? _grad_grad_vel_x[_qp].tr()
                 : (k == 2 ? _grad_grad_vel_y[_qp].tr() : _grad_grad_vel_z[_qp].tr());

      _R[_qp](k) -= _eps[_qp] * (*_mu_eff)[_qp] * grad_grad_velocity +
                    (_eps[_qp] * grad_mu_eff + (*_mu_eff)[_qp] * _grad_eps[_qp]) * grad_velocity;
    }
  }

  // In RZ coordinates, there is an extra term in the quasi-linear
  // residual due to the divergence operator in cylindrical
  // coordinates.
  if (_assembly.coordSystem() == Moose::COORD_RZ)
  {
    Real r = _q_point[_qp](_rz_coord);
    _R[_qp].add(+_eps[_qp] / r, _F[_qp][_rz_coord]);
    _R[_qp].add(-_eps[_qp] / r, _G[_qp][_rz_coord]);
  }
}

void
NavierStokesSUPGMaterial::computeQpProperties()
{
  if (_bnd)
    return;

  _vel2 = _speed[_qp] * _speed[_qp];
  _total_energy = _rho_et[_qp] / _rho[_qp];

  // Compute derivatives of pressure and temperature w.r.t. conservation variables.
  // This function fills in the _dP and _dT vectors.
  if (_fluid_energy_eqn)
    (this->*_single_phase_dP_and_dT)();

  // Compute derivatives of specific_total_enthalpy with respect to conservation variables. This
  // function fills in the _dh vector
  if (_fluid_energy_eqn)
    computeEnthalpyDerivatives();

  // compute matrices and vectors involved, described in the doc/manual and
  // doc/conserved_form_supg
  computedUdt();
  computedUdx();
  computeA();
  computeF();
  computeG();
  computeS();
  computeR();

  // uncomment for debugging
  // printMatrices();
  // printState();
}

void
NavierStokesSUPGMaterial::printMatrices()
{
  // print matrices
  Moose::out << std::endl;

  for (unsigned int i = 0; i < _mesh_dim; ++i)
  {
    Moose::out << std::endl;
    Moose::out << "A(" << i << ") = " << std::endl;
    _A[_qp][i].print(Moose::out);
  }

  // print vectors (for every spatial dimension)
  for (unsigned int i = 0; i < _mesh_dim; ++i)
  {
    Moose::out << std::endl;
    Moose::out << "F(" << i + 1 << ") = " << std::endl;
    MetaPhysicL::raw_value(_F[_qp][i]).print(Moose::out);

    Moose::out << std::endl;
    Moose::out << "G(" << i + 1 << ") = " << std::endl;
    MetaPhysicL::raw_value(_G[_qp][i]).print(Moose::out);
  }

  // print vectors (single vectors)
  Moose::out << std::endl;
  Moose::out << "S: ";
  MetaPhysicL::raw_value(_S[_qp]).print(Moose::out);
}

void
NavierStokesSUPGMaterial::printState()
{
  // print all other related problem variables for hand-checking
  Moose::out << std::endl;
  Moose::out << "vel_x = " << _velocity[_qp](0) << "\n";
  Moose::out << "vel_y = " << _velocity[_qp](1) << "\n";
  Moose::out << "vel_z = " << _velocity[_qp](2) << "\n";
  Moose::out << "rho = " << _rho[_qp] << "\n";
  Moose::out << "acceleration = (" << _acceleration(0) << ", " << _acceleration(1) << ", "
             << _acceleration(2) << ")\n";
  Moose::out << "pressure = " << _p[_qp] << "\n";
  Moose::out << "porosity = " << _eps[_qp] << "\n";
  Moose::out << "porosity_gradient = (" << _grad_eps[_qp](0) << ", " << _grad_eps[_qp](1) << ", "
             << _grad_eps[_qp](2) << ")\n";
  Moose::out << "alpha = " << (*_alpha)[_qp] << "\n";
  Moose::out << "delta_T = " << _T_fluid[_qp] - _T_solid[_qp] << "\n";
  Moose::out << "heat_source_term = " << _scaling_factor * _heat_source[_qp] << "\n";
  Moose::out << "kappa = " << (*_kappa)[_qp] << "\n";
  Moose::out << "grad_T = (" << _grad_T_fluid[_qp](0) << ", " << _grad_T_fluid[_qp](1) << ", "
             << _grad_T_fluid[_qp](2) << ")\n";
  Moose::out << "rho_et = " << _rho_et[_qp] << "\n";
  // FIXME: this won't work if we generalize this class to work with
  // both SinglePhaseFluidProperties _and_ SinglePhaseFluidPropertiesPT
  // EOS objects.
  if (_fluid != nullptr)
  {
    Moose::out << "cv = " << _fluid->cv_from_v_e(_v[_qp], _specific_total_energy[_qp]) << "\n";
    if (_fluid_ideal_gas)
      Moose::out << "gamma = " << _fluid_ideal_gas->gamma() << "\n";
  }
  Moose::out << "specific_total_enthalpy = " << _specific_total_enthalpy[_qp] << "\n";
  Moose::out << "T_fluid = " << _T_fluid[_qp] << "\n";
}

void
NavierStokesSUPGMaterial::print_numpy(const DenseMatrix<Real> & mat) const
{
  std::ios_base::fmtflags os_flags = Moose::out.flags();
  for (unsigned int i = 0; i < mat.m(); ++i)
  {
    // Start string
    Moose::out << "'";

    for (unsigned int j = 0; j < mat.n(); ++j)
      Moose::out << std::setw(15) << std::scientific << std::setprecision(16) << mat(i, j) << " ";

    // Close string, possibly with semi-colon
    if (i < mat.m() - 1)
      Moose::out << ";";
    Moose::out << "'\n";
  }
  Moose::out.flags(os_flags);
}
