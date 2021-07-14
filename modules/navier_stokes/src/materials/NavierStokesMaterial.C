//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NavierStokesMaterial.h"
#include "NS.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseMesh.h"

#include "libmesh/quadrature.h"

namespace nms = NS;

InputParameters
NavierStokesMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addClassDescription(
      "This is the base class of all materials should use if you are trying to "
      "use the Navier-Stokes Kernels.");
  params.addRequiredCoupledVar(nms::velocity_x, "x-velocity");
  params.addCoupledVar(nms::velocity_y, "y-velocity"); // only required in >= 2D
  params.addCoupledVar(nms::velocity_z, "z-velocity"); // only required in 3D

  params.addRequiredCoupledVar(nms::temperature, "temperature");
  params.addRequiredCoupledVar(nms::specific_total_enthalpy, "specific total enthalpy");

  params.addRequiredCoupledVar(nms::density, "density");
  params.addRequiredCoupledVar(nms::momentum_x, "x-momentum");
  params.addCoupledVar(nms::momentum_y, "y-momentum"); // only required in >= 2D
  params.addCoupledVar(nms::momentum_z, "z-momentum"); // only required in 3D
  params.addRequiredCoupledVar(nms::total_energy_density, "energy");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");
  return params;
}

NavierStokesMaterial::NavierStokesMaterial(const InputParameters & parameters)
  : Material(parameters),
    _mesh_dimension(_mesh.dimension()),
    _grad_u(coupledGradient(nms::velocity_x)),
    _grad_v(_mesh_dimension >= 2 ? coupledGradient(nms::velocity_y) : _grad_zero),
    _grad_w(_mesh_dimension == 3 ? coupledGradient(nms::velocity_z) : _grad_zero),

    _viscous_stress_tensor(declareProperty<RealTensorValue>("viscous_stress_tensor")),
    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),

    // Declared here but _not_ calculated here
    // (See e.g. derived class, bighorn/include/materials/FluidTC1.h)
    _dynamic_viscosity(declareProperty<Real>("dynamic_viscosity")),

    // The momentum components of the inviscid flux Jacobians.
    _calA(declareProperty<std::vector<RealTensorValue>>("calA")),

    // "Velocity column" matrices
    _calC(declareProperty<std::vector<RealTensorValue>>("calC")),

    // Energy equation inviscid flux matrices, "cal E_{kl}" in the notes.
    _calE(declareProperty<std::vector<std::vector<RealTensorValue>>>("calE")),
    _vel_grads({&_grad_u, &_grad_v, &_grad_w}),

    // Coupled solution values needed for computing SUPG stabilization terms
    _u_vel(coupledValue(nms::velocity_x)),
    _v_vel(_mesh.dimension() >= 2 ? coupledValue(nms::velocity_y) : _zero),
    _w_vel(_mesh.dimension() == 3 ? coupledValue(nms::velocity_z) : _zero),

    _temperature(coupledValue(nms::temperature)),
    _specific_total_enthalpy(coupledValue(nms::specific_total_enthalpy)),

    // Coupled solution values
    _rho(coupledValue(nms::density)),
    _rho_u(coupledValue(nms::momentum_x)),
    _rho_v(_mesh.dimension() >= 2 ? coupledValue(nms::momentum_y) : _zero),
    _rho_w(_mesh.dimension() == 3 ? coupledValue(nms::momentum_z) : _zero),
    _rho_et(coupledValue(nms::total_energy_density)),

    // Time derivative values
    _drho_dt(coupledDot(nms::density)),
    _drhou_dt(coupledDot(nms::momentum_x)),
    _drhov_dt(_mesh.dimension() >= 2 ? coupledDot(nms::momentum_y) : _zero),
    _drhow_dt(_mesh.dimension() == 3 ? coupledDot(nms::momentum_z) : _zero),
    _drhoE_dt(coupledDot(nms::total_energy_density)),

    // Gradients
    _grad_rho(coupledGradient(nms::density)),
    _grad_rho_u(coupledGradient(nms::momentum_x)),
    _grad_rho_v(_mesh.dimension() >= 2 ? coupledGradient(nms::momentum_y) : _grad_zero),
    _grad_rho_w(_mesh.dimension() == 3 ? coupledGradient(nms::momentum_z) : _grad_zero),
    _grad_rho_et(coupledGradient(nms::total_energy_density)),

    // Material properties for stabilization
    _hsupg(declareProperty<Real>("hsupg")),
    _tauc(declareProperty<Real>("tauc")),
    _taum(declareProperty<Real>("taum")),
    _taue(declareProperty<Real>("taue")),
    _strong_residuals(declareProperty<std::vector<Real>>("strong_residuals")),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

/**
 * Must be called _after_ the child class computes dynamic_viscosity.
 */
void
NavierStokesMaterial::computeProperties()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    /******* Viscous Stress Tensor *******/
    // Technically... this _is_ the transpose (since we are loading these by rows)
    // But it doesn't matter....
    RealTensorValue grad_outer_u(_grad_u[qp], _grad_v[qp], _grad_w[qp]);

    grad_outer_u += grad_outer_u.transpose();

    Real div_vel = 0.0;
    for (unsigned int i = 0; i < 3; ++i)
      div_vel += (*_vel_grads[i])[qp](i);

    // Add diagonal terms
    for (unsigned int i = 0; i < 3; ++i)
      grad_outer_u(i, i) -= 2.0 / 3.0 * div_vel;

    grad_outer_u *= _dynamic_viscosity[qp];

    _viscous_stress_tensor[qp] = grad_outer_u;

    // Tabulated values of thermal conductivity vs. Temperature for air (k increases slightly with
    // T):
    // T (K)    k (W/m-K)
    // 273      0.0243
    // 373      0.0314
    // 473      0.0386
    // 573      0.0454
    // 673      0.0515

    // Pr = (mu * cp) / k  ==>  k = (mu * cp) / Pr = (mu * gamma * cv) / Pr.
    // TODO: We are using a fixed value of the Prandtl number which is
    // valid for air, it may or may not depend on temperature?  Since
    // this is a property of the fluid, it could possibly be moved to
    // the FluidProperties module...
    const Real Pr = 0.71;
    _thermal_conductivity[qp] = (_dynamic_viscosity[qp] * _fp.cp()) / Pr;

    // Compute stabilization parameters:

    // .) Compute SUPG element length scale.
    computeHSUPG(qp);
    // Moose::out << "_hsupg[" << qp << "]=" << _hsupg[qp] << std::endl;

    // .) Compute SUPG parameter values.  (Must call this after computeHSUPG())
    computeTau(qp);
    // Moose::out << "_tauc[" << qp << "]=" << _tauc[qp] << ", ";
    // Moose::out << "_taum[" << qp << "]=" << _taum[qp] << ", ";
    // Moose::out << "_taue[" << qp << "]=" << _taue[qp] << std::endl;

    // .) Compute strong residual values.
    computeStrongResiduals(qp);
    // Moose::out << "_strong_residuals[" << qp << "]=";
    // for (unsigned i=0; i<_strong_residuals[qp].size(); ++i)
    //   Moose::out << _strong_residuals[qp][i] << " ";
    // Moose::out << std::endl;
  }
}

void
NavierStokesMaterial::computeHSUPG(unsigned int qp)
{
  // // Grab reference to linear Lagrange finite element object pointer,
  // // currently this is always a linear Lagrange element, so this might need to
  // // be generalized if we start working with higher-order elements...
  // FEBase*& fe(_assembly.getFE(FEType(), _current_elem->dim()));
  //
  // // Grab references to FE object's mapping data from the _subproblem's FE object
  // const std::vector<Real> & dxidx(fe->get_dxidx());
  // const std::vector<Real> & dxidy(fe->get_dxidy());
  // const std::vector<Real> & dxidz(fe->get_dxidz());
  // const std::vector<Real> & detadx(fe->get_detadx());
  // const std::vector<Real> & detady(fe->get_detady());
  // const std::vector<Real> & detadz(fe->get_detadz());
  // const std::vector<Real> & dzetadx(fe->get_dzetadx()); // Empty in 2D
  // const std::vector<Real> & dzetady(fe->get_dzetady()); // Empty in 2D
  // const std::vector<Real> & dzetadz(fe->get_dzetadz()); // Empty in 2D
  //
  // // Bounds checking on element data
  // mooseAssert(qp < dxidx.size(), "Insufficient data in dxidx array!");
  // mooseAssert(qp < dxidy.size(), "Insufficient data in dxidy array!");
  // mooseAssert(qp < dxidz.size(), "Insufficient data in dxidz array!");
  //
  // mooseAssert(qp < detadx.size(), "Insufficient data in detadx array!");
  // mooseAssert(qp < detady.size(), "Insufficient data in detady array!");
  // mooseAssert(qp < detadz.size(), "Insufficient data in detadz array!");
  //
  // if (_mesh_dimension == 3)
  // {
  //   mooseAssert(qp < dzetadx.size(), "Insufficient data in dzetadx array!");
  //   mooseAssert(qp < dzetady.size(), "Insufficient data in dzetady array!");
  //   mooseAssert(qp < dzetadz.size(), "Insufficient data in dzetadz array!");
  // }
  //
  // // The velocity vector at this quadrature point.
  // RealVectorValue U(_u_vel[qp],_v_vel[qp],_w_vel[qp]);
  //
  // // Pull out element inverse map values at the current qp into a little dense matrix
  // Real dxi_dx[3][3] = {{0.,0.,0.}, {0.,0.,0.}, {0.,0.,0.}};
  //
  // dxi_dx[0][0] = dxidx[qp];  dxi_dx[0][1] = dxidy[qp];
  // dxi_dx[1][0] = detadx[qp]; dxi_dx[1][1] = detady[qp];
  //
  // // OK to access third entries on 2D elements if LIBMESH_DIM==3, though they
  // // may be zero...
  // if (LIBMESH_DIM == 3)
  // {
  //   /**/             /**/               dxi_dx[0][2] = dxidz[qp];
  //   /**/             /**/               dxi_dx[1][2] = detadz[qp];
  // }
  //
  // // The last row of entries available only for 3D elements.
  // if (_mesh_dimension == 3)
  // {
  //   dxi_dx[2][0] = dzetadx[qp];   dxi_dx[2][1] = dzetady[qp];   dxi_dx[2][2] = dzetadz[qp];
  // }
  //
  // // Construct the g_ij = d(xi_k)/d(x_j) * d(xi_k)/d(x_i) matrix
  // // from Ben and Bova's paper by summing over k...
  // Real g[3][3] = {{0.,0.,0.}, {0.,0.,0.}, {0.,0.,0.}};
  // for (unsigned int i = 0; i < 3; ++i)
  //   for (unsigned int j = 0; j < 3; ++j)
  //     for (unsigned int k = 0; k < 3; ++k)
  //       g[i][j] += dxi_dx[k][j] * dxi_dx[k][i];
  //
  // // Compute the denominator of the h_supg term: U * (g) * U
  // Real denom = 0.;
  // for (unsigned int i = 0; i < 3; ++i)
  //   for (unsigned int j = 0; j < 3; ++j)
  //     denom += U(j) * g[i][j] * U(i);
  //
  // // Compute h_supg.  Some notes:
  // // .) The 2 coefficient in this term should be a 1 if we are using tets/triangles.
  // // .) The denominator will be identically zero only if the velocity
  // //    is identically zero, in which case we can't divide by it.
  // if (denom != 0.0)
  //   _hsupg[qp] = 2.* sqrt( U.norm_sq() / denom );
  // else
  //   _hsupg[qp] = 0.;

  // Simple (and fast) implementation: Just use hmin for the element!
  _hsupg[qp] = _current_elem->hmin();
}

void
NavierStokesMaterial::computeTau(unsigned int qp)
{
  Real velmag =
      std::sqrt(_u_vel[qp] * _u_vel[qp] + _v_vel[qp] * _v_vel[qp] + _w_vel[qp] * _w_vel[qp]);

  // Moose::out << "velmag=" << velmag << std::endl;

  // Make sure temperature >= 0 before trying to take sqrt
  // if (_temperature[qp] < 0.)
  // {
  //   Moose::err << "Negative temperature "
  //             << _temperature[qp]
  //             << " found at quadrature point "
  //             << qp
  //             << ", element "
  //             << _current_elem->id()
  //             << std::endl;
  //   mooseError("Can't continue, would be nice to throw an exception here?");
  // }

  // The speed of sound for an ideal gas, sqrt(gamma * R * T).  Not needed unless
  // we want to use a form of Tau that requires it.
  // Real soundspeed = _fp.c_from_v_e(_specific_volume[_qp], _internal_energy[_qp]);

  // If velmag == 0, then _hsupg should be zero as well.  Then tau
  // will have only the time-derivative contribution (or zero, if we
  // are not including dt terms in our taus!)  Note that using the
  // time derivative contribution in this way assumes we are solving
  // unsteady, and guarantees *some* stabilization is added even when
  // u -> 0 in certain regions of the flow.
  if (velmag == 0.)
  {
    // 1.) Tau without dt terms
    // _tauc[qp] = 0.;
    // _taum[qp] = 0.;
    // _taue[qp] = 0.;

    // 2.) Tau *with* dt terms
    _tauc[qp] = _taum[qp] = _taue[qp] = 0.5 * _dt;
  }
  else
  {
    // The element length parameter, squared
    Real h2 = _hsupg[qp] * _hsupg[qp];

    // The viscosity-based term
    Real visc_term = _dynamic_viscosity[qp] / _rho[qp] / h2;

    // The thermal conductivity-based term, cp = gamma * cv
    Real k_term = _thermal_conductivity[qp] / _rho[qp] / _fp.cp() / h2;

    // 1a.) Standard compressible flow tau.  Does not account for low Mach number
    // limit.
    //    _tauc[qp] = _hsupg[qp] / (velmag + soundspeed);

    // 1b.) Inspired by Hauke, the sum of the compressible and incompressible tauc.
    //    _tauc[qp] =
    //      _hsupg[qp] / (velmag + soundspeed) +
    //      _hsupg[qp] / (velmag);

    // 1c.) From Wong 2001.  This tau is O(M^2) for small M.  At small M,
    // tauc dominates the inverse square sums and basically makes
    // taum=taue=tauc.  However, all my flows occur at low Mach numbers,
    // so there would basically never be any stabilization...
    // _tauc[qp] = (_hsupg[qp] * velmag) / (velmag*velmag + soundspeed*soundspeed);

    // For use with option "1",
    // (tau_c)^{-2}
    //    Real taucm2 = 1./_tauc[qp]/_tauc[qp];
    //    _taum[qp] = 1. / std::sqrt(taucm2 + visc_term*visc_term);
    //    _taue[qp] = 1. / std::sqrt(taucm2 + k_term*k_term);

    // 2.) Tau with timestep dependence (guarantees stabilization even
    // in zero-velocity limit) incorporated via the "r-switch" method,
    // with r=2.
    Real sqrt_term = 4. / _dt / _dt + velmag * velmag / h2;

    // For use with option "2", i.e. the option that uses dt in the definition of tau
    _tauc[qp] = 1. / std::sqrt(sqrt_term);
    _taum[qp] = 1. / std::sqrt(sqrt_term + visc_term * visc_term);
    _taue[qp] = 1. / std::sqrt(sqrt_term + k_term * k_term);
  }

  // Debugging
  // Moose::out << "_tauc[" << qp << "]=" << _tauc[qp] << std::endl;
  // Moose::out << "_hsupg[" << qp << "]=" << _hsupg[qp] << std::endl;
  // Moose::out << "velmag[" << qp << "]=" << velmag << std::endl;
}

void
NavierStokesMaterial::computeStrongResiduals(unsigned int qp)
{
  // Create storage at this qp for the strong residuals of all the equations.
  // In 2D, the value for the z-velocity equation will just be zero.
  _strong_residuals[qp].resize(5);

  // The timestep is stored in the Problem object, which can be accessed through
  // the parent pointer of the SubProblem.  Don't need this if we are not
  // approximating time derivatives ourselves.
  // Real dt = _subproblem.parent()->dt();
  // Moose::out << "dt=" << dt << std::endl;

  // Vector object for the velocity
  RealVectorValue vel(_u_vel[qp], _v_vel[qp], _w_vel[qp]);

  // A VectorValue object containing all zeros.  Makes it easier to
  // construct type tensor objects
  RealVectorValue zero(0., 0., 0.);

  // Velocity vector magnitude squared
  Real velmag2 = vel.norm_sq();

  // Debugging: How large are the time derivative parts of the strong residuals?
  //  Moose::out << "drho_dt=" << _drho_dt
  //            << ", drhou_dt=" << _drhou_dt
  //            << ", drhov_dt=" << _drhov_dt
  //            << ", drhow_dt=" << _drhow_dt
  //            << ", drhoE_dt=" << _drhoE_dt
  //            << std::endl;

  // Momentum divergence
  Real divU = _grad_rho_u[qp](0) + _grad_rho_v[qp](1) + _grad_rho_w[qp](2);

  // Enough space to hold three space dimensions of velocity components at each qp,
  // regardless of what dimension we are actually running in.
  _calC[qp].resize(3);

  // Explicitly zero the calC
  for (unsigned int i = 0; i < 3; ++i)
    _calC[qp][i].zero();

  // x-column matrix
  _calC[qp][0](0, 0) = _u_vel[qp];
  _calC[qp][0](1, 0) = _v_vel[qp];
  _calC[qp][0](2, 0) = _w_vel[qp];

  // y-column matrix
  _calC[qp][1](0, 1) = _u_vel[qp];
  _calC[qp][1](1, 1) = _v_vel[qp];
  _calC[qp][1](2, 1) = _w_vel[qp];

  // z-column matrix (this assumes LIBMESH_DIM==3!)
  _calC[qp][2](0, 2) = _u_vel[qp];
  _calC[qp][2](1, 2) = _v_vel[qp];
  _calC[qp][2](2, 2) = _w_vel[qp];

  // The matrix S can be computed from any of the calC via calC_1*calC_1^T
  RealTensorValue calS = _calC[qp][0] * _calC[qp][0].transpose();

  // Enough space to hold five (=n_sd + 2) 3*3 calA matrices at this qp, regarless of dimension
  _calA[qp].resize(5);

  // 0.) _calA_0 = diag( (gam - 1)/2*|u|^2 ) - S
  _calA[qp][0].zero(); // zero this calA entry
  _calA[qp][0](0, 0) = _calA[qp][0](1, 1) = _calA[qp][0](2, 2) =
      0.5 * (_fp.gamma() - 1.0) * velmag2; // set diag. entries
  _calA[qp][0] -= calS;

  for (unsigned int m = 1; m <= 3; ++m)
  {
    // Use m_local when indexing into matrices and vectors
    unsigned int m_local = m - 1;

    // For m=1,2,3, calA_m = C_m + C_m^T + diag( (1.-gam)*u_m )
    _calA[qp][m].zero(); // zero this calA entry
    _calA[qp][m](0, 0) = _calA[qp][m](1, 1) = _calA[qp][m](2, 2) =
        (1. - _fp.gamma()) * vel(m_local);          // set diag. entries
    _calA[qp][m] += _calC[qp][m_local];             // Note: use m_local for indexing into _calC!
    _calA[qp][m] += _calC[qp][m_local].transpose(); // Note: use m_local for indexing into _calC!
  }

  // 4.) calA_4 = diag(gam - 1)
  _calA[qp][4].zero(); // zero this calA entry
  _calA[qp][4](0, 0) = _calA[qp][4](1, 1) = _calA[qp][4](2, 2) = (_fp.gamma() - 1.0);

  // Enough space to hold the 3*5 "cal E" matrices which comprise the inviscid flux term
  // of the energy equation.  See notes for additional details
  _calE[qp].resize(3); // Three rows, 5 entries in each row

  for (unsigned int k = 0; k < 3; ++k)
  {
    // Make enough room to store all 5 E matrices for this k
    _calE[qp][k].resize(5);

    // Store and reuse the velocity column transpose matrix for the
    // current value of k.
    RealTensorValue Ck_T = _calC[qp][k].transpose();

    // E_{k0} (density gradient term)
    _calE[qp][k][0].zero();
    _calE[qp][k][0] = (0.5 * (_fp.gamma() - 1.0) * velmag2 - _specific_total_enthalpy[qp]) * Ck_T;

    for (unsigned int m = 1; m <= 3; ++m)
    {
      // Use m_local when indexing into matrices and vectors
      unsigned int m_local = m - 1;

      // E_{km} (momentum gradient terms)
      _calE[qp][k][m].zero();
      _calE[qp][k][m](k, m_local) = _specific_total_enthalpy[qp];  // H * D_{km}
      _calE[qp][k][m] += (1. - _fp.gamma()) * vel(m_local) * Ck_T; // (1-gam) * u_m * C_k^T
    }

    // E_{k4} (energy gradient term)
    _calE[qp][k][4].zero();
    _calE[qp][k][4] = _fp.gamma() * Ck_T;
  }

  // Compute the sum over ell of: A_ell grad(U_ell), store in DenseVector or Gradient object?
  // The gradient object might be more useful, since we are multiplying by VariableGradient
  // (which is a MooseArray of RealGradients) objects?
  RealVectorValue mom_resid = _calA[qp][0] * _grad_rho[qp] + _calA[qp][1] * _grad_rho_u[qp] +
                              _calA[qp][2] * _grad_rho_v[qp] + _calA[qp][3] * _grad_rho_w[qp] +
                              _calA[qp][4] * _grad_rho_et[qp];

  // No matrices/vectors for the energy residual strong form... just write it out like
  // the mass equation residual.  See "Momentum SUPG terms prop. to energy residual"
  // section of the notes.
  Real energy_resid =
      (0.5 * (_fp.gamma() - 1.0) * velmag2 - _specific_total_enthalpy[qp]) * (vel * _grad_rho[qp]) +
      _specific_total_enthalpy[qp] * divU +
      (1. - _fp.gamma()) * (vel(0) * (vel * _grad_rho_u[qp]) + vel(1) * (vel * _grad_rho_v[qp]) +
                            vel(2) * (vel * _grad_rho_w[qp])) +
      _fp.gamma() * (vel * _grad_rho_et[qp]);

  // Now for the actual residual values...

  // The density strong-residual
  _strong_residuals[qp][0] = _drho_dt[qp] + divU;

  // The x-momentum strong-residual, viscous terms neglected.
  // TODO: If we want to add viscous contributions back in, should this kernel
  // not inherit from NSViscousFluxBase so it can get tau values?  This would
  // also involve shape function second derivative values.
  _strong_residuals[qp][1] = _drhou_dt[qp] + mom_resid(0);

  // The y-momentum strong residual, viscous terms neglected.
  _strong_residuals[qp][2] = _drhov_dt[qp] + mom_resid(1);

  // The z-momentum strong residual, viscous terms neglected.
  if (_mesh_dimension == 3)
    _strong_residuals[qp][3] = _drhow_dt[qp] + mom_resid(2);
  else
    _strong_residuals[qp][3] = 0.;

  // The energy equation strong residual
  _strong_residuals[qp][4] = _drhoE_dt[qp] + energy_resid;
}
