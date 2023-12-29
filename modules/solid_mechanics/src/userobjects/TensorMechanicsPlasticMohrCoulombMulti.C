//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TensorMechanicsPlasticMohrCoulombMulti.h"
#include "RankFourTensor.h"

// Following is for perturbing eigvenvalues.  This looks really bodgy, but works quite well!
#include "MooseRandom.h"
#include "libmesh/utility.h"

registerMooseObject("TensorMechanicsApp", TensorMechanicsPlasticMohrCoulombMulti);

InputParameters
TensorMechanicsPlasticMohrCoulombMulti::validParams()
{
  InputParameters params = TensorMechanicsPlasticModel::validParams();
  params.addClassDescription("Non-associative Mohr-Coulomb plasticity with hardening/softening");
  params.addRequiredParam<UserObjectName>(
      "cohesion", "A TensorMechanicsHardening UserObject that defines hardening of the cohesion");
  params.addRequiredParam<UserObjectName>("friction_angle",
                                          "A TensorMechanicsHardening UserObject "
                                          "that defines hardening of the "
                                          "friction angle (in radians)");
  params.addRequiredParam<UserObjectName>("dilation_angle",
                                          "A TensorMechanicsHardening UserObject "
                                          "that defines hardening of the "
                                          "dilation angle (in radians)");
  params.addParam<unsigned int>("max_iterations",
                                10,
                                "Maximum number of Newton-Raphson iterations "
                                "allowed in the custom return-map algorithm. "
                                " For highly nonlinear hardening this may "
                                "need to be higher than 10.");
  params.addParam<Real>("shift",
                        "Yield surface is shifted by this amount to avoid problems with "
                        "defining derivatives when eigenvalues are equal.  If this is "
                        "larger than f_tol, a warning will be issued.  This may be set "
                        "very small when using the custom returnMap.  Default = f_tol.");
  params.addParam<bool>("use_custom_returnMap",
                        true,
                        "Use a custom return-map algorithm for this "
                        "plasticity model, which may speed up "
                        "computations considerably.  Set to true "
                        "only for isotropic elasticity with no "
                        "hardening of the dilation angle.  In this "
                        "case you may set 'shift' very small.");

  return params;
}

TensorMechanicsPlasticMohrCoulombMulti::TensorMechanicsPlasticMohrCoulombMulti(
    const InputParameters & parameters)
  : TensorMechanicsPlasticModel(parameters),
    _cohesion(getUserObject<TensorMechanicsHardeningModel>("cohesion")),
    _phi(getUserObject<TensorMechanicsHardeningModel>("friction_angle")),
    _psi(getUserObject<TensorMechanicsHardeningModel>("dilation_angle")),
    _max_iters(getParam<unsigned int>("max_iterations")),
    _shift(parameters.isParamValid("shift") ? getParam<Real>("shift") : _f_tol),
    _use_custom_returnMap(getParam<bool>("use_custom_returnMap"))
{
  if (_shift < 0)
    mooseError("Value of 'shift' in TensorMechanicsPlasticMohrCoulombMulti must not be negative\n");
  if (_shift > _f_tol)
    _console << "WARNING: value of 'shift' in TensorMechanicsPlasticMohrCoulombMulti is probably "
                "set too high"
             << std::endl;
  if (LIBMESH_DIM != 3)
    mooseError("TensorMechanicsPlasticMohrCoulombMulti is only defined for LIBMESH_DIM=3");
  MooseRandom::seed(0);
}

unsigned int
TensorMechanicsPlasticMohrCoulombMulti::numberSurfaces() const
{
  return 6;
}

void
TensorMechanicsPlasticMohrCoulombMulti::yieldFunctionV(const RankTwoTensor & stress,
                                                       Real intnl,
                                                       std::vector<Real> & f) const
{
  std::vector<Real> eigvals;
  stress.symmetricEigenvalues(eigvals);
  eigvals[0] += _shift;
  eigvals[2] -= _shift;

  const Real sinphi = std::sin(phi(intnl));
  const Real cosphi = std::cos(phi(intnl));
  const Real cohcos = cohesion(intnl) * cosphi;

  yieldFunctionEigvals(eigvals[0], eigvals[1], eigvals[2], sinphi, cohcos, f);
}

void
TensorMechanicsPlasticMohrCoulombMulti::yieldFunctionEigvals(
    Real e0, Real e1, Real e2, Real sinphi, Real cohcos, std::vector<Real> & f) const
{
  // Naively it seems a shame to have 6 yield functions active instead of just
  // 3.  But 3 won't do.  Eg, think of a loading with eigvals[0]=eigvals[1]=eigvals[2]
  // Then to return to the yield surface would require 2 positive plastic multipliers
  // and one negative one.  Boo hoo.

  f.resize(6);
  f[0] = 0.5 * (e0 - e1) + 0.5 * (e0 + e1) * sinphi - cohcos;
  f[1] = 0.5 * (e1 - e0) + 0.5 * (e0 + e1) * sinphi - cohcos;
  f[2] = 0.5 * (e0 - e2) + 0.5 * (e0 + e2) * sinphi - cohcos;
  f[3] = 0.5 * (e2 - e0) + 0.5 * (e0 + e2) * sinphi - cohcos;
  f[4] = 0.5 * (e1 - e2) + 0.5 * (e1 + e2) * sinphi - cohcos;
  f[5] = 0.5 * (e2 - e1) + 0.5 * (e1 + e2) * sinphi - cohcos;
}

void
TensorMechanicsPlasticMohrCoulombMulti::perturbStress(const RankTwoTensor & stress,
                                                      std::vector<Real> & eigvals,
                                                      std::vector<RankTwoTensor> & deigvals) const
{
  Real small_perturbation;
  RankTwoTensor shifted_stress = stress;
  while (eigvals[0] > eigvals[1] - 0.1 * _shift || eigvals[1] > eigvals[2] - 0.1 * _shift)
  {
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j <= i; ++j)
      {
        small_perturbation = 0.1 * _shift * 2 * (MooseRandom::rand() - 0.5);
        shifted_stress(i, j) += small_perturbation;
        shifted_stress(j, i) += small_perturbation;
      }
    shifted_stress.dsymmetricEigenvalues(eigvals, deigvals);
  }
}

void
TensorMechanicsPlasticMohrCoulombMulti::df_dsig(const RankTwoTensor & stress,
                                                Real sin_angle,
                                                std::vector<RankTwoTensor> & df) const
{
  std::vector<Real> eigvals;
  std::vector<RankTwoTensor> deigvals;
  stress.dsymmetricEigenvalues(eigvals, deigvals);

  if (eigvals[0] > eigvals[1] - 0.1 * _shift || eigvals[1] > eigvals[2] - 0.1 * _shift)
    perturbStress(stress, eigvals, deigvals);

  df.resize(6);
  df[0] = 0.5 * (deigvals[0] - deigvals[1]) + 0.5 * (deigvals[0] + deigvals[1]) * sin_angle;
  df[1] = 0.5 * (deigvals[1] - deigvals[0]) + 0.5 * (deigvals[0] + deigvals[1]) * sin_angle;
  df[2] = 0.5 * (deigvals[0] - deigvals[2]) + 0.5 * (deigvals[0] + deigvals[2]) * sin_angle;
  df[3] = 0.5 * (deigvals[2] - deigvals[0]) + 0.5 * (deigvals[0] + deigvals[2]) * sin_angle;
  df[4] = 0.5 * (deigvals[1] - deigvals[2]) + 0.5 * (deigvals[1] + deigvals[2]) * sin_angle;
  df[5] = 0.5 * (deigvals[2] - deigvals[1]) + 0.5 * (deigvals[1] + deigvals[2]) * sin_angle;
}

void
TensorMechanicsPlasticMohrCoulombMulti::dyieldFunction_dstressV(
    const RankTwoTensor & stress, Real intnl, std::vector<RankTwoTensor> & df_dstress) const
{
  const Real sinphi = std::sin(phi(intnl));
  df_dsig(stress, sinphi, df_dstress);
}

void
TensorMechanicsPlasticMohrCoulombMulti::dyieldFunction_dintnlV(const RankTwoTensor & stress,
                                                               Real intnl,
                                                               std::vector<Real> & df_dintnl) const
{
  std::vector<Real> eigvals;
  stress.symmetricEigenvalues(eigvals);
  eigvals[0] += _shift;
  eigvals[2] -= _shift;

  const Real sin_angle = std::sin(phi(intnl));
  const Real cos_angle = std::cos(phi(intnl));
  const Real dsin_angle = cos_angle * dphi(intnl);
  const Real dcos_angle = -sin_angle * dphi(intnl);
  const Real dcohcos = dcohesion(intnl) * cos_angle + cohesion(intnl) * dcos_angle;

  df_dintnl.resize(6);
  df_dintnl[0] = df_dintnl[1] = 0.5 * (eigvals[0] + eigvals[1]) * dsin_angle - dcohcos;
  df_dintnl[2] = df_dintnl[3] = 0.5 * (eigvals[0] + eigvals[2]) * dsin_angle - dcohcos;
  df_dintnl[4] = df_dintnl[5] = 0.5 * (eigvals[1] + eigvals[2]) * dsin_angle - dcohcos;
}

void
TensorMechanicsPlasticMohrCoulombMulti::flowPotentialV(const RankTwoTensor & stress,
                                                       Real intnl,
                                                       std::vector<RankTwoTensor> & r) const
{
  const Real sinpsi = std::sin(psi(intnl));
  df_dsig(stress, sinpsi, r);
}

void
TensorMechanicsPlasticMohrCoulombMulti::dflowPotential_dstressV(
    const RankTwoTensor & stress, Real intnl, std::vector<RankFourTensor> & dr_dstress) const
{
  std::vector<RankFourTensor> d2eigvals;
  stress.d2symmetricEigenvalues(d2eigvals);

  const Real sinpsi = std::sin(psi(intnl));

  dr_dstress.resize(6);
  dr_dstress[0] =
      0.5 * (d2eigvals[0] - d2eigvals[1]) + 0.5 * (d2eigvals[0] + d2eigvals[1]) * sinpsi;
  dr_dstress[1] =
      0.5 * (d2eigvals[1] - d2eigvals[0]) + 0.5 * (d2eigvals[0] + d2eigvals[1]) * sinpsi;
  dr_dstress[2] =
      0.5 * (d2eigvals[0] - d2eigvals[2]) + 0.5 * (d2eigvals[0] + d2eigvals[2]) * sinpsi;
  dr_dstress[3] =
      0.5 * (d2eigvals[2] - d2eigvals[0]) + 0.5 * (d2eigvals[0] + d2eigvals[2]) * sinpsi;
  dr_dstress[4] =
      0.5 * (d2eigvals[1] - d2eigvals[2]) + 0.5 * (d2eigvals[1] + d2eigvals[2]) * sinpsi;
  dr_dstress[5] =
      0.5 * (d2eigvals[2] - d2eigvals[1]) + 0.5 * (d2eigvals[1] + d2eigvals[2]) * sinpsi;
}

void
TensorMechanicsPlasticMohrCoulombMulti::dflowPotential_dintnlV(
    const RankTwoTensor & stress, Real intnl, std::vector<RankTwoTensor> & dr_dintnl) const
{
  const Real cos_angle = std::cos(psi(intnl));
  const Real dsin_angle = cos_angle * dpsi(intnl);

  std::vector<Real> eigvals;
  std::vector<RankTwoTensor> deigvals;
  stress.dsymmetricEigenvalues(eigvals, deigvals);

  if (eigvals[0] > eigvals[1] - 0.1 * _shift || eigvals[1] > eigvals[2] - 0.1 * _shift)
    perturbStress(stress, eigvals, deigvals);

  dr_dintnl.resize(6);
  dr_dintnl[0] = dr_dintnl[1] = 0.5 * (deigvals[0] + deigvals[1]) * dsin_angle;
  dr_dintnl[2] = dr_dintnl[3] = 0.5 * (deigvals[0] + deigvals[2]) * dsin_angle;
  dr_dintnl[4] = dr_dintnl[5] = 0.5 * (deigvals[1] + deigvals[2]) * dsin_angle;
}

void
TensorMechanicsPlasticMohrCoulombMulti::activeConstraints(const std::vector<Real> & f,
                                                          const RankTwoTensor & stress,
                                                          Real intnl,
                                                          const RankFourTensor & Eijkl,
                                                          std::vector<bool> & act,
                                                          RankTwoTensor & returned_stress) const
{
  act.assign(6, false);

  if (f[0] <= _f_tol && f[1] <= _f_tol && f[2] <= _f_tol && f[3] <= _f_tol && f[4] <= _f_tol &&
      f[5] <= _f_tol)
  {
    returned_stress = stress;
    return;
  }

  Real returned_intnl;
  std::vector<Real> dpm(6);
  RankTwoTensor delta_dp;
  std::vector<Real> yf(6);
  bool trial_stress_inadmissible;
  doReturnMap(stress,
              intnl,
              Eijkl,
              0.0,
              returned_stress,
              returned_intnl,
              dpm,
              delta_dp,
              yf,
              trial_stress_inadmissible);

  for (unsigned i = 0; i < 6; ++i)
    act[i] = (dpm[i] > 0);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::cohesion(const Real internal_param) const
{
  return _cohesion.value(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::dcohesion(const Real internal_param) const
{
  return _cohesion.derivative(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::phi(const Real internal_param) const
{
  return _phi.value(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::dphi(const Real internal_param) const
{
  return _phi.derivative(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::psi(const Real internal_param) const
{
  return _psi.value(internal_param);
}

Real
TensorMechanicsPlasticMohrCoulombMulti::dpsi(const Real internal_param) const
{
  return _psi.derivative(internal_param);
}

std::string
TensorMechanicsPlasticMohrCoulombMulti::modelName() const
{
  return "MohrCoulombMulti";
}

bool
TensorMechanicsPlasticMohrCoulombMulti::returnMap(const RankTwoTensor & trial_stress,
                                                  Real intnl_old,
                                                  const RankFourTensor & E_ijkl,
                                                  Real ep_plastic_tolerance,
                                                  RankTwoTensor & returned_stress,
                                                  Real & returned_intnl,
                                                  std::vector<Real> & dpm,
                                                  RankTwoTensor & delta_dp,
                                                  std::vector<Real> & yf,
                                                  bool & trial_stress_inadmissible) const
{
  if (!_use_custom_returnMap)
    return TensorMechanicsPlasticModel::returnMap(trial_stress,
                                                  intnl_old,
                                                  E_ijkl,
                                                  ep_plastic_tolerance,
                                                  returned_stress,
                                                  returned_intnl,
                                                  dpm,
                                                  delta_dp,
                                                  yf,
                                                  trial_stress_inadmissible);

  return doReturnMap(trial_stress,
                     intnl_old,
                     E_ijkl,
                     ep_plastic_tolerance,
                     returned_stress,
                     returned_intnl,
                     dpm,
                     delta_dp,
                     yf,
                     trial_stress_inadmissible);
}

bool
TensorMechanicsPlasticMohrCoulombMulti::doReturnMap(const RankTwoTensor & trial_stress,
                                                    Real intnl_old,
                                                    const RankFourTensor & E_ijkl,
                                                    Real ep_plastic_tolerance,
                                                    RankTwoTensor & returned_stress,
                                                    Real & returned_intnl,
                                                    std::vector<Real> & dpm,
                                                    RankTwoTensor & delta_dp,
                                                    std::vector<Real> & yf,
                                                    bool & trial_stress_inadmissible) const
{
  mooseAssert(dpm.size() == 6,
              "TensorMechanicsPlasticMohrCoulombMulti size of dpm should be 6 but it is "
                  << dpm.size());

  std::vector<Real> eigvals;
  RankTwoTensor eigvecs;
  trial_stress.symmetricEigenvaluesEigenvectors(eigvals, eigvecs);
  eigvals[0] += _shift;
  eigvals[2] -= _shift;

  Real sinphi = std::sin(phi(intnl_old));
  Real cosphi = std::cos(phi(intnl_old));
  Real coh = cohesion(intnl_old);
  Real cohcos = coh * cosphi;

  yieldFunctionEigvals(eigvals[0], eigvals[1], eigvals[2], sinphi, cohcos, yf);

  if (yf[0] <= _f_tol && yf[1] <= _f_tol && yf[2] <= _f_tol && yf[3] <= _f_tol && yf[4] <= _f_tol &&
      yf[5] <= _f_tol)
  {
    // purely elastic (trial_stress, intnl_old)
    trial_stress_inadmissible = false;
    return true;
  }

  trial_stress_inadmissible = true;
  delta_dp.zero();
  returned_stress = RankTwoTensor();

  // these are the normals to the 6 yield surfaces, which are const because of the assumption of no
  // psi hardening
  std::vector<RealVectorValue> norm(6);
  const Real sinpsi = std::sin(psi(intnl_old));
  const Real oneminus = 0.5 * (1 - sinpsi);
  const Real oneplus = 0.5 * (1 + sinpsi);
  norm[0](0) = oneplus;
  norm[0](1) = -oneminus;
  norm[0](2) = 0;
  norm[1](0) = -oneminus;
  norm[1](1) = oneplus;
  norm[1](2) = 0;
  norm[2](0) = oneplus;
  norm[2](1) = 0;
  norm[2](2) = -oneminus;
  norm[3](0) = -oneminus;
  norm[3](1) = 0;
  norm[3](2) = oneplus;
  norm[4](0) = 0;
  norm[4](1) = oneplus;
  norm[4](2) = -oneminus;
  norm[5](0) = 0;
  norm[5](1) = -oneminus;
  norm[5](2) = oneplus;

  // the flow directions are these norm multiplied by Eijkl.
  // I call the flow directions "n".
  // In the following I assume that the Eijkl is
  // for an isotropic situation.  Then I don't have to
  // rotate to the principal-stress frame, and i don't
  // have to worry about strange off-diagonal things
  std::vector<RealVectorValue> n(6);
  for (unsigned ys = 0; ys < 6; ++ys)
    for (unsigned i = 0; i < 3; ++i)
      for (unsigned j = 0; j < 3; ++j)
        n[ys](i) += E_ijkl(i, i, j, j) * norm[ys](j);
  const Real mag_E = E_ijkl(0, 0, 0, 0);

  // With non-zero Poisson's ratio and hardening
  // it is not computationally cheap to know whether
  // the trial stress will return to the tip, edge,
  // or plane.  The following at least
  // gives a not-completely-stupid guess
  // trial_order[0] = type of return to try first
  // trial_order[1] = type of return to try second
  // trial_order[2] = type of return to try third
  // trial_order[3] = type of return to try fourth
  // trial_order[4] = type of return to try fifth
  // In the following the "binary" stuff indicates the
  // deactive (0) and active (1) surfaces, eg
  // 110100 means that surfaces 0, 1 and 3 are active
  // and 2, 4 and 5 are deactive
  const unsigned int number_of_return_paths = 5;
  std::vector<int> trial_order(number_of_return_paths);
  if (yf[1] > _f_tol && yf[3] > _f_tol && yf[5] > _f_tol)
  {
    trial_order[0] = tip110100;
    trial_order[1] = edge010100;
    trial_order[2] = plane000100;
    trial_order[3] = edge000101;
    trial_order[4] = tip010101;
  }
  else if (yf[1] <= _f_tol && yf[3] > _f_tol && yf[5] > _f_tol)
  {
    trial_order[0] = edge000101;
    trial_order[1] = plane000100;
    trial_order[2] = tip110100;
    trial_order[3] = tip010101;
    trial_order[4] = edge010100;
  }
  else if (yf[1] <= _f_tol && yf[3] > _f_tol && yf[5] <= _f_tol)
  {
    trial_order[0] = plane000100;
    trial_order[1] = edge000101;
    trial_order[2] = edge010100;
    trial_order[3] = tip110100;
    trial_order[4] = tip010101;
  }
  else
  {
    trial_order[0] = edge010100;
    trial_order[1] = plane000100;
    trial_order[2] = edge000101;
    trial_order[3] = tip110100;
    trial_order[4] = tip010101;
  }

  unsigned trial;
  bool nr_converged = false;
  bool kt_success = false;
  std::vector<RealVectorValue> ntip(3);
  std::vector<Real> dpmtip(3);

  for (trial = 0; trial < number_of_return_paths; ++trial)
  {
    switch (trial_order[trial])
    {
      case tip110100:
        for (unsigned int i = 0; i < 3; ++i)
        {
          ntip[0](i) = n[0](i);
          ntip[1](i) = n[1](i);
          ntip[2](i) = n[3](i);
        }
        kt_success = returnTip(eigvals,
                               ntip,
                               dpmtip,
                               returned_stress,
                               intnl_old,
                               sinphi,
                               cohcos,
                               0,
                               nr_converged,
                               ep_plastic_tolerance,
                               yf);
        if (nr_converged && kt_success)
        {
          dpm[0] = dpmtip[0];
          dpm[1] = dpmtip[1];
          dpm[3] = dpmtip[2];
          dpm[2] = dpm[4] = dpm[5] = 0;
        }
        break;

      case tip010101:
        for (unsigned int i = 0; i < 3; ++i)
        {
          ntip[0](i) = n[1](i);
          ntip[1](i) = n[3](i);
          ntip[2](i) = n[5](i);
        }
        kt_success = returnTip(eigvals,
                               ntip,
                               dpmtip,
                               returned_stress,
                               intnl_old,
                               sinphi,
                               cohcos,
                               0,
                               nr_converged,
                               ep_plastic_tolerance,
                               yf);
        if (nr_converged && kt_success)
        {
          dpm[1] = dpmtip[0];
          dpm[3] = dpmtip[1];
          dpm[5] = dpmtip[2];
          dpm[0] = dpm[2] = dpm[4] = 0;
        }
        break;

      case edge000101:
        kt_success = returnEdge000101(eigvals,
                                      n,
                                      dpm,
                                      returned_stress,
                                      intnl_old,
                                      sinphi,
                                      cohcos,
                                      0,
                                      mag_E,
                                      nr_converged,
                                      ep_plastic_tolerance,
                                      yf);
        break;

      case edge010100:
        kt_success = returnEdge010100(eigvals,
                                      n,
                                      dpm,
                                      returned_stress,
                                      intnl_old,
                                      sinphi,
                                      cohcos,
                                      0,
                                      mag_E,
                                      nr_converged,
                                      ep_plastic_tolerance,
                                      yf);
        break;

      case plane000100:
        kt_success = returnPlane(eigvals,
                                 n,
                                 dpm,
                                 returned_stress,
                                 intnl_old,
                                 sinphi,
                                 cohcos,
                                 0,
                                 nr_converged,
                                 ep_plastic_tolerance,
                                 yf);
        break;
    }

    if (nr_converged && kt_success)
      break;
  }

  if (trial == number_of_return_paths)
  {
    sinphi = std::sin(phi(intnl_old));
    cosphi = std::cos(phi(intnl_old));
    coh = cohesion(intnl_old);
    cohcos = coh * cosphi;
    yieldFunctionEigvals(eigvals[0], eigvals[1], eigvals[2], sinphi, cohcos, yf);
    Moose::err << "Trial stress = \n";
    trial_stress.print(Moose::err);
    Moose::err << "which has eigenvalues = " << eigvals[0] << " " << eigvals[1] << " " << eigvals[2]
               << "\n";
    Moose::err << "and yield functions = " << yf[0] << " " << yf[1] << " " << yf[2] << " " << yf[3]
               << " " << yf[4] << " " << yf[5] << "\n";
    Moose::err << "Internal parameter = " << intnl_old << std::endl;
    mooseError("TensorMechanicsPlasticMohrCoulombMulti: FAILURE!  You probably need to implement a "
               "line search if your hardening is too severe, or you need to tune your tolerances "
               "(eg, yield_function_tolerance should be a little smaller than (young "
               "modulus)*ep_plastic_tolerance).\n");
    return false;
  }

  // success

  returned_intnl = intnl_old;
  for (unsigned i = 0; i < 6; ++i)
    returned_intnl += dpm[i];
  for (unsigned i = 0; i < 6; ++i)
    for (unsigned j = 0; j < 3; ++j)
      delta_dp(j, j) += dpm[i] * norm[i](j);
  returned_stress = eigvecs * returned_stress * (eigvecs.transpose());
  delta_dp = eigvecs * delta_dp * (eigvecs.transpose());
  return true;
}

bool
TensorMechanicsPlasticMohrCoulombMulti::returnTip(const std::vector<Real> & eigvals,
                                                  const std::vector<RealVectorValue> & n,
                                                  std::vector<Real> & dpm,
                                                  RankTwoTensor & returned_stress,
                                                  Real intnl_old,
                                                  Real & sinphi,
                                                  Real & cohcos,
                                                  Real initial_guess,
                                                  bool & nr_converged,
                                                  Real ep_plastic_tolerance,
                                                  std::vector<Real> & yf) const
{
  // This returns to the Mohr-Coulomb tip using the THREE directions
  // given in n, and yields the THREE dpm values.  Note that you
  // must supply THREE suitable n vectors out of the total of SIX
  // flow directions, and then interpret the THREE dpm values appropriately.
  //
  // Eg1.  You supply the flow directions n[0], n[1] and n[3] as
  //       the "n" vectors.  This is return-to-the-tip via 110100.
  //       Then the three returned dpm values will be dpm[0], dpm[1] and dpm[3].

  // Eg2.  You supply the flow directions n[1], n[3] and n[5] as
  //       the "n" vectors.  This is return-to-the-tip via 010101.
  //       Then the three returned dpm values will be dpm[1], dpm[3] and dpm[5].

  // The returned point is defined by the three yield functions (corresonding
  // to the three supplied flow directions) all being zero.
  // that is, returned_stress = diag(cohcot, cohcot, cohcot), where
  // cohcot = cohesion*cosphi/sinphi
  // where intnl = intnl_old + dpm[0] + dpm[1] + dpm[2]
  // The 3 plastic multipliers, dpm, are defiend by the normality condition
  //     eigvals - cohcot = dpm[0]*n[0] + dpm[1]*n[1] + dpm[2]*n[2]
  // (Kuhn-Tucker demands that all dpm are non-negative, but we leave
  //  that checking for the end.)
  // (Remember that these "n" vectors and "dpm" values must be interpreted
  //  differently depending on what you pass into this function.)
  // This is a vector equation with solution (A):
  //   dpm[0] = triple(eigvals - cohcot, n[1], n[2])/trip;
  //   dpm[1] = triple(eigvals - cohcot, n[2], n[0])/trip;
  //   dpm[2] = triple(eigvals - cohcot, n[0], n[1])/trip;
  // where trip = triple(n[0], n[1], n[2]).
  // By adding the three components of that solution together
  // we can get an equation for x = dpm[0] + dpm[1] + dpm[2],
  // and then our Newton-Raphson only involves one variable (x).
  // In the following, i specialise to the isotropic situation.

  mooseAssert(n.size() == 3,
              "TensorMechanicsPlasticMohrCoulombMulti: Custom tip-return algorithm must be "
              "supplied with n of size 3, whereas yours is "
                  << n.size());
  mooseAssert(dpm.size() == 3,
              "TensorMechanicsPlasticMohrCoulombMulti: Custom tip-return algorithm must be "
              "supplied with dpm of size 3, whereas yours is "
                  << dpm.size());
  mooseAssert(yf.size() == 6,
              "TensorMechanicsPlasticMohrCoulombMulti: Custom tip-return algorithm must be "
              "supplied with yf of size 6, whereas yours is "
                  << yf.size());

  Real x = initial_guess;
  const Real trip = triple_product(n[0], n[1], n[2]);
  sinphi = std::sin(phi(intnl_old + x));
  Real cosphi = std::cos(phi(intnl_old + x));
  Real coh = cohesion(intnl_old + x);
  cohcos = coh * cosphi;
  Real cohcot = cohcos / sinphi;

  if (_cohesion.modelName().compare("Constant") != 0 || _phi.modelName().compare("Constant") != 0)
  {
    // Finding x is expensive.  Therefore
    // although x!=0 for Constant Hardening, solution (A)
    // demonstrates that we don't
    // actually need to know x to find the dpm for
    // Constant Hardening.
    //
    // However, for nontrivial Hardening, the following
    // is necessary
    // cohcot_coeff = [1,1,1].(Cross[n[1], n[2]] + Cross[n[2], n[0]] + Cross[n[0], n[1]])/trip
    Real cohcot_coeff =
        (n[0](0) * (n[1](1) - n[1](2) - n[2](1)) + (n[1](2) - n[1](1)) * n[2](0) +
         (n[1](0) - n[1](2)) * n[2](1) + n[0](2) * (n[1](0) - n[1](1) - n[2](0) + n[2](1)) +
         n[0](1) * (n[1](2) - n[1](0) + n[2](0) - n[2](2)) +
         (n[0](0) - n[1](0) + n[1](1)) * n[2](2)) /
        trip;
    // eig_term = eigvals.(Cross[n[1], n[2]] + Cross[n[2], n[0]] + Cross[n[0], n[1]])/trip
    Real eig_term = eigvals[0] *
                    (-n[0](2) * n[1](1) + n[0](1) * n[1](2) + n[0](2) * n[2](1) -
                     n[1](2) * n[2](1) - n[0](1) * n[2](2) + n[1](1) * n[2](2)) /
                    trip;
    eig_term += eigvals[1] *
                (n[0](2) * n[1](0) - n[0](0) * n[1](2) - n[0](2) * n[2](0) + n[1](2) * n[2](0) +
                 n[0](0) * n[2](2) - n[1](0) * n[2](2)) /
                trip;
    eig_term += eigvals[2] *
                (n[0](0) * n[1](1) - n[1](1) * n[2](0) + n[0](1) * n[2](0) - n[0](1) * n[1](0) -
                 n[0](0) * n[2](1) + n[1](0) * n[2](1)) /
                trip;
    // and finally, the equation we want to solve is:
    // x - eig_term + cohcot*cohcot_coeff = 0
    // but i divide by cohcot_coeff so the result has the units of
    // stress, so using _f_tol as a convergence check is reasonable
    eig_term /= cohcot_coeff;
    Real residual = x / cohcot_coeff - eig_term + cohcot;
    Real jacobian;
    Real deriv_phi;
    Real deriv_coh;
    unsigned int iter = 0;
    do
    {
      deriv_phi = dphi(intnl_old + x);
      deriv_coh = dcohesion(intnl_old + x);
      jacobian = 1.0 / cohcot_coeff + deriv_coh * cosphi / sinphi -
                 coh * deriv_phi / Utility::pow<2>(sinphi);
      x += -residual / jacobian;

      if (iter > _max_iters) // not converging
      {
        nr_converged = false;
        return false;
      }

      sinphi = std::sin(phi(intnl_old + x));
      cosphi = std::cos(phi(intnl_old + x));
      coh = cohesion(intnl_old + x);
      cohcos = coh * cosphi;
      cohcot = cohcos / sinphi;
      residual = x / cohcot_coeff - eig_term + cohcot;
      iter++;
    } while (residual * residual > _f_tol * _f_tol / 100);
  }

  // so the NR process converged, but we must
  // calculate the individual dpm values and
  // check Kuhn-Tucker
  nr_converged = true;
  if (x < -3 * ep_plastic_tolerance)
    // obviously at least one of the dpm are < -ep_plastic_tolerance.  No point in proceeding.  This
    // is a potential weak-point: if the user has set _f_tol quite large, and ep_plastic_tolerance
    // quite small, the above NR process will quickly converge, but the solution may be wrong and
    // violate Kuhn-Tucker.
    return false;

  // The following is the solution (A) written above
  // (dpm[0] = triple(eigvals - cohcot, n[1], n[2])/trip, etc)
  // in the isotropic situation
  RealVectorValue v;
  v(0) = eigvals[0] - cohcot;
  v(1) = eigvals[1] - cohcot;
  v(2) = eigvals[2] - cohcot;
  dpm[0] = triple_product(v, n[1], n[2]) / trip;
  dpm[1] = triple_product(v, n[2], n[0]) / trip;
  dpm[2] = triple_product(v, n[0], n[1]) / trip;

  if (dpm[0] < -ep_plastic_tolerance || dpm[1] < -ep_plastic_tolerance ||
      dpm[2] < -ep_plastic_tolerance)
    // Kuhn-Tucker failure.  No point in proceeding
    return false;

  // Kuhn-Tucker has succeeded: just need returned_stress and yf values
  // I do not use the dpm to calculate returned_stress, because that
  // might add error (and computational effort), simply:
  returned_stress(0, 0) = returned_stress(1, 1) = returned_stress(2, 2) = cohcot;
  // So by construction the yield functions are all zero
  yf[0] = yf[1] = yf[2] = yf[3] = yf[4] = yf[5] = 0;
  return true;
}

bool
TensorMechanicsPlasticMohrCoulombMulti::returnPlane(const std::vector<Real> & eigvals,
                                                    const std::vector<RealVectorValue> & n,
                                                    std::vector<Real> & dpm,
                                                    RankTwoTensor & returned_stress,
                                                    Real intnl_old,
                                                    Real & sinphi,
                                                    Real & cohcos,
                                                    Real initial_guess,
                                                    bool & nr_converged,
                                                    Real ep_plastic_tolerance,
                                                    std::vector<Real> & yf) const
{
  // This returns to the Mohr-Coulomb plane using n[3] (ie 000100)
  //
  // The returned point is defined by the f[3]=0 and
  //    a = eigvals - dpm[3]*n[3]
  // where "a" is the returned point and dpm[3] is the plastic multiplier.
  // This equation is a vector equation in principal stress space.
  // (Kuhn-Tucker also demands that dpm[3]>=0, but we leave checking
  // that condition for the end.)
  // Since f[3]=0, we must have
  // a[2]*(1+sinphi) + a[0]*(-1+sinphi) - 2*coh*cosphi = 0
  // which gives dpm[3] as the solution of
  //     alpha*dpm[3] + eigvals[2] - eigvals[0] + beta*sinphi - 2*coh*cosphi = 0
  // with alpha = n[3](0) - n[3](2) - (n[3](2) + n[3](0))*sinphi
  //      beta = eigvals[2] + eigvals[0]

  mooseAssert(n.size() == 6,
              "TensorMechanicsPlasticMohrCoulombMulti: Custom plane-return algorithm must be "
              "supplied with n of size 6, whereas yours is "
                  << n.size());
  mooseAssert(dpm.size() == 6,
              "TensorMechanicsPlasticMohrCoulombMulti: Custom plane-return algorithm must be "
              "supplied with dpm of size 6, whereas yours is "
                  << dpm.size());
  mooseAssert(yf.size() == 6,
              "TensorMechanicsPlasticMohrCoulombMulti: Custom tip-return algorithm must be "
              "supplied with yf of size 6, whereas yours is "
                  << yf.size());

  dpm[3] = initial_guess;
  sinphi = std::sin(phi(intnl_old + dpm[3]));
  Real cosphi = std::cos(phi(intnl_old + dpm[3]));
  Real coh = cohesion(intnl_old + dpm[3]);
  cohcos = coh * cosphi;

  Real alpha = n[3](0) - n[3](2) - (n[3](2) + n[3](0)) * sinphi;
  Real deriv_phi;
  Real dalpha;
  const Real beta = eigvals[2] + eigvals[0];
  Real deriv_coh;

  Real residual =
      alpha * dpm[3] + eigvals[2] - eigvals[0] + beta * sinphi - 2.0 * cohcos; // this is 2*yf[3]
  Real jacobian;

  const Real f_tol2 = Utility::pow<2>(_f_tol);
  unsigned int iter = 0;
  do
  {
    deriv_phi = dphi(intnl_old + dpm[3]);
    dalpha = -(n[3](2) + n[3](0)) * cosphi * deriv_phi;
    deriv_coh = dcohesion(intnl_old + dpm[3]);
    jacobian = alpha + dalpha * dpm[3] + beta * cosphi * deriv_phi - 2.0 * deriv_coh * cosphi +
               2.0 * coh * sinphi * deriv_phi;

    dpm[3] -= residual / jacobian;
    if (iter > _max_iters) // not converging
    {
      nr_converged = false;
      return false;
    }

    sinphi = std::sin(phi(intnl_old + dpm[3]));
    cosphi = std::cos(phi(intnl_old + dpm[3]));
    coh = cohesion(intnl_old + dpm[3]);
    cohcos = coh * cosphi;
    alpha = n[3](0) - n[3](2) - (n[3](2) + n[3](0)) * sinphi;
    residual = alpha * dpm[3] + eigvals[2] - eigvals[0] + beta * sinphi - 2.0 * cohcos;
    iter++;
  } while (residual * residual > f_tol2);

  // so the NR process converged, but we must
  // check Kuhn-Tucker
  nr_converged = true;
  if (dpm[3] < -ep_plastic_tolerance)
    // Kuhn-Tucker failure
    return false;

  for (unsigned i = 0; i < 3; ++i)
    returned_stress(i, i) = eigvals[i] - dpm[3] * n[3](i);
  yieldFunctionEigvals(
      returned_stress(0, 0), returned_stress(1, 1), returned_stress(2, 2), sinphi, cohcos, yf);

  // by construction abs(yf[3]) = abs(residual/2) < _f_tol/2
  if (yf[0] > _f_tol || yf[1] > _f_tol || yf[2] > _f_tol || yf[4] > _f_tol || yf[5] > _f_tol)
    // Kuhn-Tucker failure
    return false;

  // success!
  dpm[0] = dpm[1] = dpm[2] = dpm[4] = dpm[5] = 0;
  return true;
}

bool
TensorMechanicsPlasticMohrCoulombMulti::returnEdge000101(const std::vector<Real> & eigvals,
                                                         const std::vector<RealVectorValue> & n,
                                                         std::vector<Real> & dpm,
                                                         RankTwoTensor & returned_stress,
                                                         Real intnl_old,
                                                         Real & sinphi,
                                                         Real & cohcos,
                                                         Real initial_guess,
                                                         Real mag_E,
                                                         bool & nr_converged,
                                                         Real ep_plastic_tolerance,
                                                         std::vector<Real> & yf) const
{
  // This returns to the Mohr-Coulomb edge
  // with 000101 being active.  This means that
  // f3=f5=0.  Denoting the returned stress by "a"
  // (in principal stress space), this means that
  // a0=a1 = (2Ccosphi + a2(1+sinphi))/(sinphi-1)
  //
  // Also, a = eigvals - dpm3*n3 - dpm5*n5,
  // where the dpm are the plastic multipliers
  // and the n are the flow directions.
  //
  // Hence we have 5 equations and 5 unknowns (a,
  // dpm3 and dpm5).  Eliminating all unknowns
  // except for x = dpm3+dpm5 gives the residual below
  // (I used mathematica)

  Real x = initial_guess;
  sinphi = std::sin(phi(intnl_old + x));
  Real cosphi = std::cos(phi(intnl_old + x));
  Real coh = cohesion(intnl_old + x);
  cohcos = coh * cosphi;

  const Real numer_const =
      -n[3](2) * eigvals[0] - n[5](1) * eigvals[0] + n[5](2) * eigvals[0] + n[3](2) * eigvals[1] +
      n[5](0) * eigvals[1] - n[5](2) * eigvals[1] - n[5](0) * eigvals[2] + n[5](1) * eigvals[2] +
      n[3](0) * (-eigvals[1] + eigvals[2]) - n[3](1) * (-eigvals[0] + eigvals[2]);
  const Real numer_coeff1 = 2 * (-n[3](0) + n[3](1) + n[5](0) - n[5](1));
  const Real numer_coeff2 =
      n[5](2) * (eigvals[0] - eigvals[1]) + n[3](2) * (-eigvals[0] + eigvals[1]) +
      n[5](1) * (eigvals[0] + eigvals[2]) + (n[3](0) - n[5](0)) * (eigvals[1] + eigvals[2]) -
      n[3](1) * (eigvals[0] + eigvals[2]);
  Real numer = numer_const + numer_coeff1 * cohcos + numer_coeff2 * sinphi;
  const Real denom_const = -n[3](2) * (n[5](0) - n[5](1)) - n[3](1) * (-n[5](0) + n[5](2)) +
                           n[3](0) * (-n[5](1) + n[5](2));
  const Real denom_coeff = -n[3](2) * (n[5](0) - n[5](1)) - n[3](1) * (n[5](0) + n[5](2)) +
                           n[3](0) * (n[5](1) + n[5](2));
  Real denom = denom_const + denom_coeff * sinphi;
  Real residual = -x + numer / denom;

  Real deriv_phi;
  Real deriv_coh;
  Real jacobian;
  const Real tol = Utility::pow<2>(_f_tol / (mag_E * 10.0));
  unsigned int iter = 0;
  do
  {
    do
    {
      deriv_phi = dphi(intnl_old + dpm[3]);
      deriv_coh = dcohesion(intnl_old + dpm[3]);
      jacobian = -1 +
                 (numer_coeff1 * deriv_coh * cosphi - numer_coeff1 * coh * sinphi * deriv_phi +
                  numer_coeff2 * cosphi * deriv_phi) /
                     denom -
                 numer * denom_coeff * cosphi * deriv_phi / denom / denom;
      x -= residual / jacobian;
      if (iter > _max_iters) // not converging
      {
        nr_converged = false;
        return false;
      }

      sinphi = std::sin(phi(intnl_old + x));
      cosphi = std::cos(phi(intnl_old + x));
      coh = cohesion(intnl_old + x);
      cohcos = coh * cosphi;
      numer = numer_const + numer_coeff1 * cohcos + numer_coeff2 * sinphi;
      denom = denom_const + denom_coeff * sinphi;
      residual = -x + numer / denom;
      iter++;
    } while (residual * residual > tol);

    // now must ensure that yf[3] and yf[5] are both "zero"
    const Real dpm3minusdpm5 =
        (2.0 * (eigvals[0] - eigvals[1]) + x * (n[3](1) - n[3](0) + n[5](1) - n[5](0))) /
        (n[3](0) - n[3](1) + n[5](1) - n[5](0));
    dpm[3] = (x + dpm3minusdpm5) / 2.0;
    dpm[5] = (x - dpm3minusdpm5) / 2.0;

    for (unsigned i = 0; i < 3; ++i)
      returned_stress(i, i) = eigvals[i] - dpm[3] * n[3](i) - dpm[5] * n[5](i);
    yieldFunctionEigvals(
        returned_stress(0, 0), returned_stress(1, 1), returned_stress(2, 2), sinphi, cohcos, yf);
  } while (yf[3] * yf[3] > _f_tol * _f_tol && yf[5] * yf[5] > _f_tol * _f_tol);

  // so the NR process converged, but we must
  // check Kuhn-Tucker
  nr_converged = true;

  if (dpm[3] < -ep_plastic_tolerance || dpm[5] < -ep_plastic_tolerance)
    // Kuhn-Tucker failure.    This is a potential weak-point: if the user has set _f_tol quite
    // large, and ep_plastic_tolerance quite small, the above NR process will quickly converge, but
    // the solution may be wrong and violate Kuhn-Tucker.
    return false;

  if (yf[0] > _f_tol || yf[1] > _f_tol || yf[2] > _f_tol || yf[4] > _f_tol)
    // Kuhn-Tucker failure
    return false;

  // success
  dpm[0] = dpm[1] = dpm[2] = dpm[4] = 0;
  return true;
}

bool
TensorMechanicsPlasticMohrCoulombMulti::returnEdge010100(const std::vector<Real> & eigvals,
                                                         const std::vector<RealVectorValue> & n,
                                                         std::vector<Real> & dpm,
                                                         RankTwoTensor & returned_stress,
                                                         Real intnl_old,
                                                         Real & sinphi,
                                                         Real & cohcos,
                                                         Real initial_guess,
                                                         Real mag_E,
                                                         bool & nr_converged,
                                                         Real ep_plastic_tolerance,
                                                         std::vector<Real> & yf) const
{
  // This returns to the Mohr-Coulomb edge
  // with 010100 being active.  This means that
  // f1=f3=0.  Denoting the returned stress by "a"
  // (in principal stress space), this means that
  // a1=a2 = (2Ccosphi + a0(1-sinphi))/(sinphi+1)
  //
  // Also, a = eigvals - dpm1*n1 - dpm3*n3,
  // where the dpm are the plastic multipliers
  // and the n are the flow directions.
  //
  // Hence we have 5 equations and 5 unknowns (a,
  // dpm1 and dpm3).  Eliminating all unknowns
  // except for x = dpm1+dpm3 gives the residual below
  // (I used mathematica)

  Real x = initial_guess;
  sinphi = std::sin(phi(intnl_old + x));
  Real cosphi = std::cos(phi(intnl_old + x));
  Real coh = cohesion(intnl_old + x);
  cohcos = coh * cosphi;

  const Real numer_const = -n[1](2) * eigvals[0] - n[3](1) * eigvals[0] + n[3](2) * eigvals[0] -
                           n[1](0) * eigvals[1] + n[1](2) * eigvals[1] + n[3](0) * eigvals[1] -
                           n[3](2) * eigvals[1] + n[1](0) * eigvals[2] - n[3](0) * eigvals[2] +
                           n[3](1) * eigvals[2] - n[1](1) * (-eigvals[0] + eigvals[2]);
  const Real numer_coeff1 = 2 * (n[1](1) - n[1](2) - n[3](1) + n[3](2));
  const Real numer_coeff2 =
      n[3](2) * (-eigvals[0] - eigvals[1]) + n[1](2) * (eigvals[0] + eigvals[1]) +
      n[3](1) * (eigvals[0] + eigvals[2]) + (n[1](0) - n[3](0)) * (eigvals[1] - eigvals[2]) -
      n[1](1) * (eigvals[0] + eigvals[2]);
  Real numer = numer_const + numer_coeff1 * cohcos + numer_coeff2 * sinphi;
  const Real denom_const = -n[1](0) * (n[3](1) - n[3](2)) + n[1](2) * (-n[3](0) + n[3](1)) +
                           n[1](1) * (-n[3](2) + n[3](0));
  const Real denom_coeff =
      n[1](0) * (n[3](1) - n[3](2)) + n[1](2) * (n[3](0) + n[3](1)) - n[1](1) * (n[3](0) + n[3](2));
  Real denom = denom_const + denom_coeff * sinphi;
  Real residual = -x + numer / denom;

  Real deriv_phi;
  Real deriv_coh;
  Real jacobian;
  const Real tol = Utility::pow<2>(_f_tol / (mag_E * 10.0));
  unsigned int iter = 0;
  do
  {
    do
    {
      deriv_phi = dphi(intnl_old + dpm[3]);
      deriv_coh = dcohesion(intnl_old + dpm[3]);
      jacobian = -1 +
                 (numer_coeff1 * deriv_coh * cosphi - numer_coeff1 * coh * sinphi * deriv_phi +
                  numer_coeff2 * cosphi * deriv_phi) /
                     denom -
                 numer * denom_coeff * cosphi * deriv_phi / denom / denom;
      x -= residual / jacobian;
      if (iter > _max_iters) // not converging
      {
        nr_converged = false;
        return false;
      }

      sinphi = std::sin(phi(intnl_old + x));
      cosphi = std::cos(phi(intnl_old + x));
      coh = cohesion(intnl_old + x);
      cohcos = coh * cosphi;
      numer = numer_const + numer_coeff1 * cohcos + numer_coeff2 * sinphi;
      denom = denom_const + denom_coeff * sinphi;
      residual = -x + numer / denom;
      iter++;
    } while (residual * residual > tol);

    // now must ensure that yf[1] and yf[3] are both "zero"
    Real dpm1minusdpm3 =
        (2 * (eigvals[1] - eigvals[2]) + x * (n[1](2) - n[1](1) + n[3](2) - n[3](1))) /
        (n[1](1) - n[1](2) + n[3](2) - n[3](1));
    dpm[1] = (x + dpm1minusdpm3) / 2.0;
    dpm[3] = (x - dpm1minusdpm3) / 2.0;

    for (unsigned i = 0; i < 3; ++i)
      returned_stress(i, i) = eigvals[i] - dpm[1] * n[1](i) - dpm[3] * n[3](i);
    yieldFunctionEigvals(
        returned_stress(0, 0), returned_stress(1, 1), returned_stress(2, 2), sinphi, cohcos, yf);
  } while (yf[1] * yf[1] > _f_tol * _f_tol && yf[3] * yf[3] > _f_tol * _f_tol);

  // so the NR process converged, but we must
  // check Kuhn-Tucker
  nr_converged = true;

  if (dpm[1] < -ep_plastic_tolerance || dpm[3] < -ep_plastic_tolerance)
    // Kuhn-Tucker failure.    This is a potential weak-point: if the user has set _f_tol quite
    // large, and ep_plastic_tolerance quite small, the above NR process will quickly converge, but
    // the solution may be wrong and violate Kuhn-Tucker
    return false;

  if (yf[0] > _f_tol || yf[2] > _f_tol || yf[4] > _f_tol || yf[5] > _f_tol)
    // Kuhn-Tucker failure
    return false;

  // success
  dpm[0] = dpm[2] = dpm[4] = dpm[5] = 0;
  return true;
}

bool
TensorMechanicsPlasticMohrCoulombMulti::KuhnTuckerOK(const std::vector<Real> & yf,
                                                     const std::vector<Real> & dpm,
                                                     Real ep_plastic_tolerance) const
{
  mooseAssert(
      yf.size() == 6,
      "TensorMechanicsPlasticMohrCoulomb::KuhnTuckerOK requires yf to be size 6, but your is "
          << yf.size());
  mooseAssert(
      dpm.size() == 6,
      "TensorMechanicsPlasticMohrCoulomb::KuhnTuckerOK requires dpm to be size 6, but your is "
          << dpm.size());
  for (unsigned i = 0; i < 6; ++i)
    if (!TensorMechanicsPlasticModel::KuhnTuckerSingleSurface(yf[i], dpm[i], ep_plastic_tolerance))
      return false;
  return true;
}

bool
TensorMechanicsPlasticMohrCoulombMulti::useCustomReturnMap() const
{
  return _use_custom_returnMap;
}
