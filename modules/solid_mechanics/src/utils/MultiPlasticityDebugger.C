//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiPlasticityDebugger.h"
#include "RankFourTensor.h"
#include "libmesh/utility.h"

InputParameters
MultiPlasticityDebugger::validParams()
{
  InputParameters params = MultiPlasticityLinearSystem::validParams();
  MooseEnum debug_fspb_type("none crash jacobian jacobian_and_linear_system", "none");
  params.addParam<MooseEnum>("debug_fspb",
                             debug_fspb_type,
                             "Debug types for use by developers when creating new "
                             "plasticity models, not for general use.  2 = debug Jacobian "
                             "entries, 3 = check the entire Jacobian, and check Ax=b");
  params.addParam<RealTensorValue>("debug_jac_at_stress",
                                   RealTensorValue(),
                                   "Debug Jacobian entries at this stress.  For use by developers");
  params.addParam<std::vector<Real>>("debug_jac_at_pm",
                                     "Debug Jacobian entries at these plastic multipliers");
  params.addParam<std::vector<Real>>("debug_jac_at_intnl",
                                     "Debug Jacobian entries at these internal parameters");
  params.addParam<Real>(
      "debug_stress_change", 1.0, "Debug finite differencing parameter for the stress");
  params.addParam<std::vector<Real>>(
      "debug_pm_change", "Debug finite differencing parameters for the plastic multipliers");
  params.addParam<std::vector<Real>>(
      "debug_intnl_change", "Debug finite differencing parameters for the internal parameters");
  return params;
}

MultiPlasticityDebugger::MultiPlasticityDebugger(const MooseObject * moose_object)
  : MultiPlasticityLinearSystem(moose_object),
    _fspb_debug(_params.get<MooseEnum>("debug_fspb")),
    _fspb_debug_stress(_params.get<RealTensorValue>("debug_jac_at_stress")),
    _fspb_debug_pm(_params.get<std::vector<Real>>("debug_jac_at_pm")),
    _fspb_debug_intnl(_params.get<std::vector<Real>>("debug_jac_at_intnl")),
    _fspb_debug_stress_change(_params.get<Real>("debug_stress_change")),
    _fspb_debug_pm_change(_params.get<std::vector<Real>>("debug_pm_change")),
    _fspb_debug_intnl_change(_params.get<std::vector<Real>>("debug_intnl_change"))
{
}

void
MultiPlasticityDebugger::outputAndCheckDebugParameters()
{
  Moose::err << "Debug Parameters are as follows\n";
  Moose::err << "stress = \n";
  _fspb_debug_stress.print();

  if (_fspb_debug_pm.size() != _num_surfaces || _fspb_debug_intnl.size() != _num_models ||
      _fspb_debug_pm_change.size() != _num_surfaces ||
      _fspb_debug_intnl_change.size() != _num_models)
    mooseError("The debug parameters have the wrong size\n");

  Moose::err << "plastic multipliers =\n";
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    Moose::err << _fspb_debug_pm[surface] << "\n";

  Moose::err << "internal parameters =\n";
  for (unsigned model = 0; model < _num_models; ++model)
    Moose::err << _fspb_debug_intnl[model] << "\n";

  Moose::err << "finite-differencing parameter for stress-changes:\n"
             << _fspb_debug_stress_change << "\n";
  Moose::err << "finite-differencing parameter(s) for plastic-multiplier(s):\n";
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    Moose::err << _fspb_debug_pm_change[surface] << "\n";
  Moose::err << "finite-differencing parameter(s) for internal-parameter(s):\n";
  for (unsigned model = 0; model < _num_models; ++model)
    Moose::err << _fspb_debug_intnl_change[model] << "\n";

  Moose::err << std::flush;
}

void
MultiPlasticityDebugger::checkDerivatives()
{
  Moose::err
      << "\n\n++++++++++++++++++++++++\nChecking the derivatives\n++++++++++++++++++++++++\n";
  outputAndCheckDebugParameters();

  std::vector<bool> act;
  act.assign(_num_surfaces, true);

  Moose::err << "\ndyieldFunction_dstress.  Relative L2 norms.\n";
  std::vector<RankTwoTensor> df_dstress;
  std::vector<RankTwoTensor> fddf_dstress;
  dyieldFunction_dstress(_fspb_debug_stress, _fspb_debug_intnl, act, df_dstress);
  fddyieldFunction_dstress(_fspb_debug_stress, _fspb_debug_intnl, fddf_dstress);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    Moose::err << "surface = " << surface << " Relative L2norm = "
               << 2 * (df_dstress[surface] - fddf_dstress[surface]).L2norm() /
                      (df_dstress[surface] + fddf_dstress[surface]).L2norm()
               << "\n";
    Moose::err << "Coded:\n";
    df_dstress[surface].print();
    Moose::err << "Finite difference:\n";
    fddf_dstress[surface].print();
  }

  Moose::err << "\ndyieldFunction_dintnl.\n";
  std::vector<Real> df_dintnl;
  dyieldFunction_dintnl(_fspb_debug_stress, _fspb_debug_intnl, act, df_dintnl);
  Moose::err << "Coded:\n";
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    Moose::err << df_dintnl[surface] << " ";
  Moose::err << "\n";
  std::vector<Real> fddf_dintnl;
  fddyieldFunction_dintnl(_fspb_debug_stress, _fspb_debug_intnl, fddf_dintnl);
  Moose::err << "Finite difference:\n";
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    Moose::err << fddf_dintnl[surface] << " ";
  Moose::err << "\n";

  Moose::err << "\ndflowPotential_dstress.  Relative L2 norms.\n";
  std::vector<RankFourTensor> dr_dstress;
  std::vector<RankFourTensor> fddr_dstress;
  dflowPotential_dstress(_fspb_debug_stress, _fspb_debug_intnl, act, dr_dstress);
  fddflowPotential_dstress(_fspb_debug_stress, _fspb_debug_intnl, fddr_dstress);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    Moose::err << "surface = " << surface << " Relative L2norm = "
               << 2 * (dr_dstress[surface] - fddr_dstress[surface]).L2norm() /
                      (dr_dstress[surface] + fddr_dstress[surface]).L2norm()
               << "\n";
    Moose::err << "Coded:\n";
    dr_dstress[surface].print();
    Moose::err << "Finite difference:\n";
    fddr_dstress[surface].print();
  }

  Moose::err << "\ndflowPotential_dintnl.  Relative L2 norms.\n";
  std::vector<RankTwoTensor> dr_dintnl;
  std::vector<RankTwoTensor> fddr_dintnl;
  dflowPotential_dintnl(_fspb_debug_stress, _fspb_debug_intnl, act, dr_dintnl);
  fddflowPotential_dintnl(_fspb_debug_stress, _fspb_debug_intnl, fddr_dintnl);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    Moose::err << "surface = " << surface << " Relative L2norm = "
               << 2 * (dr_dintnl[surface] - fddr_dintnl[surface]).L2norm() /
                      (dr_dintnl[surface] + fddr_dintnl[surface]).L2norm()
               << "\n";
    Moose::err << "Coded:\n";
    dr_dintnl[surface].print();
    Moose::err << "Finite difference:\n";
    fddr_dintnl[surface].print();
  }

  Moose::err << std::flush;
}

void
MultiPlasticityDebugger::checkJacobian(const RankFourTensor & E_inv,
                                       const std::vector<Real> & intnl_old)
{
  Moose::err << "\n\n+++++++++++++++++++++\nChecking the Jacobian\n+++++++++++++++++++++\n";
  outputAndCheckDebugParameters();

  std::vector<bool> act;
  act.assign(_num_surfaces, true);
  std::vector<bool> deactivated_due_to_ld;
  deactivated_due_to_ld.assign(_num_surfaces, false);

  RankTwoTensor delta_dp = -E_inv * _fspb_debug_stress;

  std::vector<std::vector<Real>> jac;
  calculateJacobian(_fspb_debug_stress,
                    _fspb_debug_intnl,
                    _fspb_debug_pm,
                    E_inv,
                    act,
                    deactivated_due_to_ld,
                    jac);

  std::vector<std::vector<Real>> fdjac;
  fdJacobian(_fspb_debug_stress,
             intnl_old,
             _fspb_debug_intnl,
             _fspb_debug_pm,
             delta_dp,
             E_inv,
             false,
             fdjac);

  Real L2_numer = 0;
  Real L2_denom = 0;
  for (unsigned row = 0; row < jac.size(); ++row)
    for (unsigned col = 0; col < jac.size(); ++col)
    {
      L2_numer += Utility::pow<2>(jac[row][col] - fdjac[row][col]);
      L2_denom += Utility::pow<2>(jac[row][col] + fdjac[row][col]);
    }
  Moose::err << "\nRelative L2norm = " << std::sqrt(L2_numer / L2_denom) / 0.5 << "\n";

  Moose::err << "\nHand-coded Jacobian:\n";
  for (unsigned row = 0; row < jac.size(); ++row)
  {
    for (unsigned col = 0; col < jac.size(); ++col)
      Moose::err << jac[row][col] << " ";
    Moose::err << "\n";
  }

  Moose::err << "Finite difference Jacobian:\n";
  for (unsigned row = 0; row < fdjac.size(); ++row)
  {
    for (unsigned col = 0; col < fdjac.size(); ++col)
      Moose::err << fdjac[row][col] << " ";
    Moose::err << "\n";
  }

  Moose::err << std::flush;
}

void
MultiPlasticityDebugger::fdJacobian(const RankTwoTensor & stress,
                                    const std::vector<Real> & intnl_old,
                                    const std::vector<Real> & intnl,
                                    const std::vector<Real> & pm,
                                    const RankTwoTensor & delta_dp,
                                    const RankFourTensor & E_inv,
                                    bool eliminate_ld,
                                    std::vector<std::vector<Real>> & jac)
{
  std::vector<bool> active;
  active.assign(_num_surfaces, true);

  std::vector<bool> deactivated_due_to_ld;
  std::vector<bool> deactivated_due_to_ld_ep;

  std::vector<Real> orig_rhs;
  calculateRHS(stress,
               intnl_old,
               intnl,
               pm,
               delta_dp,
               orig_rhs,
               active,
               eliminate_ld,
               deactivated_due_to_ld); // this calculates RHS, and also set deactivated_due_to_ld.
                                       // The latter stays fixed for the rest of this routine

  unsigned int whole_system_size = 6 + _num_surfaces + _num_models;
  unsigned int system_size =
      orig_rhs.size(); // will be = whole_system_size if eliminate_ld = false, since all active=true
  jac.resize(system_size);
  for (unsigned row = 0; row < system_size; ++row)
    jac[row].assign(system_size, 0);

  std::vector<Real> rhs_ep;
  unsigned col = 0;

  RankTwoTensor stressep;
  RankTwoTensor delta_dpep;
  Real ep = _fspb_debug_stress_change;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j <= i; ++j)
    {
      stressep = stress;
      stressep(i, j) += ep;
      if (i != j)
        stressep(j, i) += ep;
      delta_dpep = delta_dp;
      for (unsigned k = 0; k < 3; ++k)
        for (unsigned l = 0; l < 3; ++l)
        {
          delta_dpep(k, l) -= E_inv(k, l, i, j) * ep;
          if (i != j)
            delta_dpep(k, l) -= E_inv(k, l, j, i) * ep;
        }
      active.assign(_num_surfaces, true);
      calculateRHS(stressep,
                   intnl_old,
                   intnl,
                   pm,
                   delta_dpep,
                   rhs_ep,
                   active,
                   false,
                   deactivated_due_to_ld_ep);
      unsigned row = 0;
      for (unsigned dof = 0; dof < whole_system_size; ++dof)
        if (dof_included(dof, deactivated_due_to_ld))
        {
          jac[row][col] =
              -(rhs_ep[dof] - orig_rhs[row]) / ep; // remember jacobian = -d(rhs)/d(something)
          row++;
        }
      col++; // all of the first 6 columns are dof_included since they're stresses
    }

  std::vector<Real> pmep;
  pmep.resize(_num_surfaces);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    pmep[surface] = pm[surface];
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    if (!dof_included(6 + surface, deactivated_due_to_ld))
      continue;
    ep = _fspb_debug_pm_change[surface];
    pmep[surface] += ep;
    active.assign(_num_surfaces, true);
    calculateRHS(
        stress, intnl_old, intnl, pmep, delta_dp, rhs_ep, active, false, deactivated_due_to_ld_ep);
    unsigned row = 0;
    for (unsigned dof = 0; dof < whole_system_size; ++dof)
      if (dof_included(dof, deactivated_due_to_ld))
      {
        jac[row][col] =
            -(rhs_ep[dof] - orig_rhs[row]) / ep; // remember jacobian = -d(rhs)/d(something)
        row++;
      }
    pmep[surface] -= ep;
    col++;
  }

  std::vector<Real> intnlep;
  intnlep.resize(_num_models);
  for (unsigned model = 0; model < _num_models; ++model)
    intnlep[model] = intnl[model];
  for (unsigned model = 0; model < _num_models; ++model)
  {
    if (!dof_included(6 + _num_surfaces + model, deactivated_due_to_ld))
      continue;
    ep = _fspb_debug_intnl_change[model];
    intnlep[model] += ep;
    active.assign(_num_surfaces, true);
    calculateRHS(
        stress, intnl_old, intnlep, pm, delta_dp, rhs_ep, active, false, deactivated_due_to_ld_ep);
    unsigned row = 0;
    for (unsigned dof = 0; dof < whole_system_size; ++dof)
      if (dof_included(dof, deactivated_due_to_ld))
      {
        jac[row][col] =
            -(rhs_ep[dof] - orig_rhs[row]) / ep; // remember jacobian = -d(rhs)/d(something)
        row++;
      }
    intnlep[model] -= ep;
    col++;
  }
}

bool
MultiPlasticityDebugger::dof_included(unsigned int dof,
                                      const std::vector<bool> & deactivated_due_to_ld)
{
  if (dof < unsigned(6))
    // these are the stress components
    return true;
  unsigned eff_dof = dof - 6;
  if (eff_dof < _num_surfaces)
    // these are the plastic multipliers, pm
    return !deactivated_due_to_ld[eff_dof];
  eff_dof -= _num_surfaces; // now we know the dof is an intnl parameter
  std::vector<bool> active_surface(_num_surfaces);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    active_surface[surface] = !deactivated_due_to_ld[surface];
  return anyActiveSurfaces(eff_dof, active_surface);
}

void
MultiPlasticityDebugger::checkSolution(const RankFourTensor & E_inv)
{
  Moose::err << "\n\n+++++++++++++++++++++\nChecking the Solution\n";
  Moose::err << "(Ie, checking Ax = b)\n+++++++++++++++++++++\n";
  outputAndCheckDebugParameters();

  std::vector<bool> act;
  act.assign(_num_surfaces, true);
  std::vector<bool> deactivated_due_to_ld;
  deactivated_due_to_ld.assign(_num_surfaces, false);

  RankTwoTensor delta_dp = -E_inv * _fspb_debug_stress;

  std::vector<Real> orig_rhs;
  calculateRHS(_fspb_debug_stress,
               _fspb_debug_intnl,
               _fspb_debug_intnl,
               _fspb_debug_pm,
               delta_dp,
               orig_rhs,
               act,
               true,
               deactivated_due_to_ld);

  Moose::err << "\nb = ";
  for (unsigned i = 0; i < orig_rhs.size(); ++i)
    Moose::err << orig_rhs[i] << " ";
  Moose::err << "\n\n";

  std::vector<std::vector<Real>> jac_coded;
  calculateJacobian(_fspb_debug_stress,
                    _fspb_debug_intnl,
                    _fspb_debug_pm,
                    E_inv,
                    act,
                    deactivated_due_to_ld,
                    jac_coded);

  Moose::err
      << "Before checking Ax=b is correct, check that the Jacobians given below are equal.\n";
  Moose::err
      << "The hand-coded Jacobian is used in calculating the solution 'x', given 'b' above.\n";
  Moose::err << "Note that this only includes degrees of freedom that aren't deactivated due to "
                "linear dependence.\n";
  Moose::err << "Hand-coded Jacobian:\n";
  for (unsigned row = 0; row < jac_coded.size(); ++row)
  {
    for (unsigned col = 0; col < jac_coded.size(); ++col)
      Moose::err << jac_coded[row][col] << " ";
    Moose::err << "\n";
  }

  deactivated_due_to_ld.assign(_num_surfaces,
                               false); // this potentially gets changed by nrStep, below
  RankTwoTensor dstress;
  std::vector<Real> dpm;
  std::vector<Real> dintnl;
  nrStep(_fspb_debug_stress,
         _fspb_debug_intnl,
         _fspb_debug_intnl,
         _fspb_debug_pm,
         E_inv,
         delta_dp,
         dstress,
         dpm,
         dintnl,
         act,
         deactivated_due_to_ld);

  std::vector<bool> active_not_deact(_num_surfaces);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    active_not_deact[surface] = !deactivated_due_to_ld[surface];

  std::vector<Real> x;
  x.assign(orig_rhs.size(), 0);
  unsigned ind = 0;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j <= i; ++j)
      x[ind++] = dstress(i, j);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (active_not_deact[surface])
      x[ind++] = dpm[surface];
  for (unsigned model = 0; model < _num_models; ++model)
    if (anyActiveSurfaces(model, active_not_deact))
      x[ind++] = dintnl[model];

  mooseAssert(ind == orig_rhs.size(),
              "Incorrect extracting of changes from NR solution in the "
              "finite-difference checking of nrStep");

  Moose::err << "\nThis yields x =";
  for (unsigned i = 0; i < orig_rhs.size(); ++i)
    Moose::err << x[i] << " ";
  Moose::err << "\n";

  std::vector<std::vector<Real>> jac_fd;
  fdJacobian(_fspb_debug_stress,
             _fspb_debug_intnl,
             _fspb_debug_intnl,
             _fspb_debug_pm,
             delta_dp,
             E_inv,
             true,
             jac_fd);

  Moose::err << "\nThe finite-difference Jacobian is used to multiply by this 'x',\n";
  Moose::err << "in order to check that the solution is correct\n";
  Moose::err << "Finite-difference Jacobian:\n";
  for (unsigned row = 0; row < jac_fd.size(); ++row)
  {
    for (unsigned col = 0; col < jac_fd.size(); ++col)
      Moose::err << jac_fd[row][col] << " ";
    Moose::err << "\n";
  }

  Real L2_numer = 0;
  Real L2_denom = 0;
  for (unsigned row = 0; row < jac_coded.size(); ++row)
    for (unsigned col = 0; col < jac_coded.size(); ++col)
    {
      L2_numer += Utility::pow<2>(jac_coded[row][col] - jac_fd[row][col]);
      L2_denom += Utility::pow<2>(jac_coded[row][col] + jac_fd[row][col]);
    }
  Moose::err << "Relative L2norm of the hand-coded and finite-difference Jacobian is "
             << std::sqrt(L2_numer / L2_denom) / 0.5 << "\n";

  std::vector<Real> fd_times_x;
  fd_times_x.assign(orig_rhs.size(), 0);
  for (unsigned row = 0; row < orig_rhs.size(); ++row)
    for (unsigned col = 0; col < orig_rhs.size(); ++col)
      fd_times_x[row] += jac_fd[row][col] * x[col];

  Moose::err << "\n(Finite-difference Jacobian)*x =\n";
  for (unsigned i = 0; i < orig_rhs.size(); ++i)
    Moose::err << fd_times_x[i] << " ";
  Moose::err << "\n";
  Moose::err << "Recall that b = \n";
  for (unsigned i = 0; i < orig_rhs.size(); ++i)
    Moose::err << orig_rhs[i] << " ";
  Moose::err << "\n";

  L2_numer = 0;
  L2_denom = 0;
  for (unsigned i = 0; i < orig_rhs.size(); ++i)
  {
    L2_numer += Utility::pow<2>(orig_rhs[i] - fd_times_x[i]);
    L2_denom += Utility::pow<2>(orig_rhs[i] + fd_times_x[i]);
  }
  Moose::err << "\nRelative L2norm of these is " << std::sqrt(L2_numer / L2_denom) / 0.5
             << std::endl;
}

void
MultiPlasticityDebugger::fddyieldFunction_dstress(const RankTwoTensor & stress,
                                                  const std::vector<Real> & intnl,
                                                  std::vector<RankTwoTensor> & df_dstress)
{
  df_dstress.assign(_num_surfaces, RankTwoTensor());

  std::vector<bool> act;
  act.assign(_num_surfaces, true);

  Real ep = _fspb_debug_stress_change;
  RankTwoTensor stressep;
  std::vector<Real> fep, fep_minus;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      stressep = stress;
      // do a central difference to attempt to capture discontinuities
      // such as those encountered in tensile and Mohr-Coulomb
      stressep(i, j) += ep / 2.0;
      yieldFunction(stressep, intnl, act, fep);
      stressep(i, j) -= ep;
      yieldFunction(stressep, intnl, act, fep_minus);
      for (unsigned surface = 0; surface < _num_surfaces; ++surface)
        df_dstress[surface](i, j) = (fep[surface] - fep_minus[surface]) / ep;
    }
}

void
MultiPlasticityDebugger::fddyieldFunction_dintnl(const RankTwoTensor & stress,
                                                 const std::vector<Real> & intnl,
                                                 std::vector<Real> & df_dintnl)
{
  df_dintnl.resize(_num_surfaces);

  std::vector<bool> act;
  act.assign(_num_surfaces, true);

  std::vector<Real> origf;
  yieldFunction(stress, intnl, act, origf);

  std::vector<Real> intnlep;
  intnlep.resize(_num_models);
  for (unsigned model = 0; model < _num_models; ++model)
    intnlep[model] = intnl[model];
  Real ep;
  std::vector<Real> fep;
  unsigned int model;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    model = modelNumber(surface);
    ep = _fspb_debug_intnl_change[model];
    intnlep[model] += ep;
    yieldFunction(stress, intnlep, act, fep);
    df_dintnl[surface] = (fep[surface] - origf[surface]) / ep;
    intnlep[model] -= ep;
  }
}

void
MultiPlasticityDebugger::fddflowPotential_dstress(const RankTwoTensor & stress,
                                                  const std::vector<Real> & intnl,
                                                  std::vector<RankFourTensor> & dr_dstress)
{
  dr_dstress.assign(_num_surfaces, RankFourTensor());

  std::vector<bool> act;
  act.assign(_num_surfaces, true);

  Real ep = _fspb_debug_stress_change;
  RankTwoTensor stressep;
  std::vector<RankTwoTensor> rep, rep_minus;
  for (unsigned i = 0; i < 3; ++i)
    for (unsigned j = 0; j < 3; ++j)
    {
      stressep = stress;
      // do a central difference
      stressep(i, j) += ep / 2.0;
      flowPotential(stressep, intnl, act, rep);
      stressep(i, j) -= ep;
      flowPotential(stressep, intnl, act, rep_minus);
      for (unsigned surface = 0; surface < _num_surfaces; ++surface)
        for (unsigned k = 0; k < 3; ++k)
          for (unsigned l = 0; l < 3; ++l)
            dr_dstress[surface](k, l, i, j) = (rep[surface](k, l) - rep_minus[surface](k, l)) / ep;
    }
}

void
MultiPlasticityDebugger::fddflowPotential_dintnl(const RankTwoTensor & stress,
                                                 const std::vector<Real> & intnl,
                                                 std::vector<RankTwoTensor> & dr_dintnl)
{
  dr_dintnl.resize(_num_surfaces);

  std::vector<bool> act;
  act.assign(_num_surfaces, true);

  std::vector<RankTwoTensor> origr;
  flowPotential(stress, intnl, act, origr);

  std::vector<Real> intnlep;
  intnlep.resize(_num_models);
  for (unsigned model = 0; model < _num_models; ++model)
    intnlep[model] = intnl[model];
  Real ep;
  std::vector<RankTwoTensor> rep;
  unsigned int model;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    model = modelNumber(surface);
    ep = _fspb_debug_intnl_change[model];
    intnlep[model] += ep;
    flowPotential(stress, intnlep, act, rep);
    dr_dintnl[surface] = (rep[surface] - origr[surface]) / ep;
    intnlep[model] -= ep;
  }
}
