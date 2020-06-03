//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiPlasticityLinearSystem.h"
#include "RankFourTensor.h"

// Following is for perturbing distances in eliminating linearly-dependent directions
#include "MooseRandom.h"

// Following is used to access PETSc's LAPACK routines
#include <petscblaslapack.h>

InputParameters
MultiPlasticityLinearSystem::validParams()
{
  InputParameters params = MultiPlasticityRawComponentAssembler::validParams();
  params.addRangeCheckedParam<Real>("linear_dependent",
                                    1E-4,
                                    "linear_dependent>=0 & linear_dependent<1",
                                    "Flow directions are considered linearly dependent if the "
                                    "smallest singular value is less than linear_dependent times "
                                    "the largest singular value");
  return params;
}

MultiPlasticityLinearSystem::MultiPlasticityLinearSystem(const MooseObject * moose_object)
  : MultiPlasticityRawComponentAssembler(moose_object),
    _svd_tol(_params.get<Real>("linear_dependent")),
    _min_f_tol(-1.0)
{
  for (unsigned model = 0; model < _num_models; ++model)
    if (_min_f_tol == -1.0 || _min_f_tol > _f[model]->_f_tol)
      _min_f_tol = _f[model]->_f_tol;

  MooseRandom::seed(0);
}

int
MultiPlasticityLinearSystem::singularValuesOfR(const std::vector<RankTwoTensor> & r,
                                               std::vector<Real> & s)
{
  PetscBLASInt bm = r.size();
  PetscBLASInt bn = 6;

  s.resize(std::min(bm, bn));

  // prepare for gesvd or gesdd routine provided by PETSc
  // Want to find the singular values of matrix
  //     (  r[0](0,0) r[0](0,1) r[0](0,2) r[0](1,1) r[0](1,2) r[0](2,2)  )
  //     (  r[1](0,0) r[1](0,1) r[1](0,2) r[1](1,1) r[1](1,2) r[1](2,2)  )
  // a = (  r[2](0,0) r[2](0,1) r[2](0,2) r[2](1,1) r[2](1,2) r[2](2,2)  )
  //     (  r[3](0,0) r[3](0,1) r[3](0,2) r[3](1,1) r[3](1,2) r[3](2,2)  )
  //     (  r[4](0,0) r[4](0,1) r[4](0,2) r[4](1,1) r[4](1,2) r[4](2,2)  )
  // bm = 5

  std::vector<double> a(bm * 6);
  // Fill in the a "matrix" by going down columns
  unsigned ind = 0;
  for (int col = 0; col < 3; ++col)
    for (int row = 0; row < bm; ++row)
      a[ind++] = r[row](0, col);
  for (int col = 3; col < 5; ++col)
    for (int row = 0; row < bm; ++row)
      a[ind++] = r[row](1, col - 2);
  for (int row = 0; row < bm; ++row)
    a[ind++] = r[row](2, 2);

  // u and vt are dummy variables because they won't
  // get referenced due to the "N" and "N" choices
  PetscBLASInt sizeu = 1;
  std::vector<double> u(sizeu);
  PetscBLASInt sizevt = 1;
  std::vector<double> vt(sizevt);

  PetscBLASInt sizework =
      16 * (bm + 6); // this is above the lowerbound specified in the LAPACK doco
  std::vector<double> work(sizework);

  PetscBLASInt info;

  LAPACKgesvd_("N",
               "N",
               &bm,
               &bn,
               &a[0],
               &bm,
               &s[0],
               &u[0],
               &sizeu,
               &vt[0],
               &sizevt,
               &work[0],
               &sizework,
               &info);

  return info;
}

void
MultiPlasticityLinearSystem::eliminateLinearDependence(const RankTwoTensor & stress,
                                                       const std::vector<Real> & intnl,
                                                       const std::vector<Real> & f,
                                                       const std::vector<RankTwoTensor> & r,
                                                       const std::vector<bool> & active,
                                                       std::vector<bool> & deactivated_due_to_ld)
{
  deactivated_due_to_ld.resize(_num_surfaces, false);

  unsigned num_active = r.size();

  if (num_active <= 1)
    return;

  std::vector<double> s;
  int info = singularValuesOfR(r, s);
  if (info != 0)
    mooseError("In finding the SVD in the return-map algorithm, the PETSC LAPACK gesvd routine "
               "returned with error code ",
               info);

  // num_lin_dep are the number of linearly dependent
  // "r vectors", if num_active <= 6
  unsigned int num_lin_dep = 0;

  unsigned i = s.size();
  while (i-- > 0)
    if (s[i] < _svd_tol * s[0])
      num_lin_dep++;
    else
      break;

  if (num_lin_dep == 0 && num_active <= 6)
    return;

  // From here on, some flow directions are linearly dependent

  // Find the signed "distance" of the current (stress, internal) configuration
  // from the yield surfaces.  This distance will not be precise, but
  // i want to preferentially deactivate yield surfaces that are close
  // to the current stress point.
  std::vector<RankTwoTensor> df_dstress;
  dyieldFunction_dstress(stress, intnl, active, df_dstress);

  typedef std::pair<Real, unsigned> pair_for_sorting;
  std::vector<pair_for_sorting> dist(num_active);
  for (unsigned i = 0; i < num_active; ++i)
  {
    dist[i].first = f[i] / df_dstress[i].L2norm();
    dist[i].second = i;
  }
  std::sort(dist.begin(), dist.end()); // sorted in ascending order

  // There is a potential problem when we have equal f[i], for it can give oscillations
  bool equals_detected = false;
  for (unsigned i = 0; i < num_active - 1; ++i)
    if (std::abs(dist[i].first - dist[i + 1].first) < _min_f_tol)
    {
      equals_detected = true;
      dist[i].first += _min_f_tol * (MooseRandom::rand() - 0.5);
    }
  if (equals_detected)
    std::sort(dist.begin(), dist.end()); // sorted in ascending order

  std::vector<bool> scheduled_for_deactivation;
  scheduled_for_deactivation.assign(num_active, false);

  // In the following loop we go through all the flow directions, from
  // the one with the largest dist, to the one with the smallest dist,
  // adding them one-by-one into r_tmp.  Upon each addition we check
  // for linear-dependence.  if LD is found, we schedule the most
  // recently added flow direction for deactivation, and pop it
  // back off r_tmp
  unsigned current_yf;
  current_yf = dist[num_active - 1].second;
  // the one with largest dist
  std::vector<RankTwoTensor> r_tmp = {r[current_yf]};

  unsigned num_kept_active = 1;
  for (unsigned yf_to_try = 2; yf_to_try <= num_active; ++yf_to_try)
  {
    current_yf = dist[num_active - yf_to_try].second;
    if (num_active == 2) // shortcut to we don't have to singularValuesOfR
      scheduled_for_deactivation[current_yf] = true;
    else if (num_kept_active >= 6) // shortcut to we don't have to singularValuesOfR: there can
                                   // never be > 6 linearly-independent r vectors
      scheduled_for_deactivation[current_yf] = true;
    else
    {
      r_tmp.push_back(r[current_yf]);
      info = singularValuesOfR(r_tmp, s);
      if (info != 0)
        mooseError("In finding the SVD in the return-map algorithm, the PETSC LAPACK gesvd routine "
                   "returned with error code ",
                   info);
      if (s[s.size() - 1] < _svd_tol * s[0])
      {
        scheduled_for_deactivation[current_yf] = true;
        r_tmp.pop_back();
        num_lin_dep--;
      }
      else
        num_kept_active++;
      if (num_lin_dep == 0 && num_active <= 6)
        // have taken out all the vectors that were linearly dependent
        // so no point continuing
        break;
    }
  }

  unsigned int old_active_number = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (active[surface])
    {
      if (scheduled_for_deactivation[old_active_number])
        deactivated_due_to_ld[surface] = true;
      old_active_number++;
    }
}

void
MultiPlasticityLinearSystem::calculateConstraints(const RankTwoTensor & stress,
                                                  const std::vector<Real> & intnl_old,
                                                  const std::vector<Real> & intnl,
                                                  const std::vector<Real> & pm,
                                                  const RankTwoTensor & delta_dp,
                                                  std::vector<Real> & f,
                                                  std::vector<RankTwoTensor> & r,
                                                  RankTwoTensor & epp,
                                                  std::vector<Real> & ic,
                                                  const std::vector<bool> & active)
{
  // see comments at the start of .h file

  mooseAssert(intnl_old.size() == _num_models,
              "Size of intnl_old is " << intnl_old.size()
                                      << " which is incorrect in calculateConstraints");
  mooseAssert(intnl.size() == _num_models,
              "Size of intnl is " << intnl.size() << " which is incorrect in calculateConstraints");
  mooseAssert(pm.size() == _num_surfaces,
              "Size of pm is " << pm.size() << " which is incorrect in calculateConstraints");
  mooseAssert(active.size() == _num_surfaces,
              "Size of active is " << active.size()
                                   << " which is incorrect in calculateConstraints");

  // yield functions
  yieldFunction(stress, intnl, active, f);

  // flow directions and "epp"
  flowPotential(stress, intnl, active, r);
  epp = RankTwoTensor();
  unsigned ind = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (active[surface])
      epp += pm[surface] * r[ind++]; // note, even the deactivated_due_to_ld must get added in
  epp -= delta_dp;

  // internal constraints
  std::vector<Real> h;
  hardPotential(stress, intnl, active, h);
  ic.resize(0);
  ind = 0;
  std::vector<unsigned int> active_surfaces;
  std::vector<unsigned int>::iterator active_surface;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    activeSurfaces(model, active, active_surfaces);
    if (active_surfaces.size() > 0)
    {
      // some surfaces are active in this model, so must form an internal constraint
      ic.push_back(intnl[model] - intnl_old[model]);
      for (active_surface = active_surfaces.begin(); active_surface != active_surfaces.end();
           ++active_surface)
        ic[ic.size() - 1] += pm[*active_surface] * h[ind++]; // we know the correct one is h[ind]
                                                             // since it was constructed in the same
                                                             // manner
    }
  }
}

void
MultiPlasticityLinearSystem::calculateRHS(const RankTwoTensor & stress,
                                          const std::vector<Real> & intnl_old,
                                          const std::vector<Real> & intnl,
                                          const std::vector<Real> & pm,
                                          const RankTwoTensor & delta_dp,
                                          std::vector<Real> & rhs,
                                          const std::vector<bool> & active,
                                          bool eliminate_ld,
                                          std::vector<bool> & deactivated_due_to_ld)
{
  // see comments at the start of .h file

  mooseAssert(intnl_old.size() == _num_models,
              "Size of intnl_old is " << intnl_old.size() << " which is incorrect in calculateRHS");
  mooseAssert(intnl.size() == _num_models,
              "Size of intnl is " << intnl.size() << " which is incorrect in calculateRHS");
  mooseAssert(pm.size() == _num_surfaces,
              "Size of pm is " << pm.size() << " which is incorrect in calculateRHS");
  mooseAssert(active.size() == _num_surfaces,
              "Size of active is " << active.size() << " which is incorrect in calculateRHS");

  std::vector<Real> f;  // the yield functions
  RankTwoTensor epp;    // the plastic-strain constraint ("direction constraint")
  std::vector<Real> ic; // the "internal constraints"

  std::vector<RankTwoTensor> r;
  calculateConstraints(stress, intnl_old, intnl, pm, delta_dp, f, r, epp, ic, active);

  if (eliminate_ld)
    eliminateLinearDependence(stress, intnl, f, r, active, deactivated_due_to_ld);
  else
    deactivated_due_to_ld.assign(_num_surfaces, false);

  std::vector<bool> active_not_deact(_num_surfaces);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    active_not_deact[surface] = (active[surface] && !deactivated_due_to_ld[surface]);

  unsigned num_active_f = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (active_not_deact[surface])
      num_active_f++;

  unsigned num_active_ic = 0;
  for (unsigned model = 0; model < _num_models; ++model)
    if (anyActiveSurfaces(model, active_not_deact))
      num_active_ic++;

  unsigned int dim = 3;
  unsigned int system_size = 6 + num_active_f + num_active_ic; // "6" comes from symmeterizing epp,
                                                               // num_active_f comes from "f",
                                                               // num_active_f comes from "ic"

  rhs.resize(system_size);

  unsigned ind = 0;
  for (unsigned i = 0; i < dim; ++i)
    for (unsigned j = 0; j <= i; ++j)
      rhs[ind++] = -epp(i, j);
  unsigned active_surface = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (active[surface])
    {
      if (!deactivated_due_to_ld[surface])
        rhs[ind++] = -f[active_surface];
      active_surface++;
    }
  unsigned active_model = 0;
  for (unsigned model = 0; model < _num_models; ++model)
    if (anyActiveSurfaces(model, active))
    {
      if (anyActiveSurfaces(model, active_not_deact))
        rhs[ind++] = -ic[active_model];
      active_model++;
    }

  mooseAssert(ind == system_size, "Incorrect filling of the rhs in calculateRHS");
}

void
MultiPlasticityLinearSystem::calculateJacobian(const RankTwoTensor & stress,
                                               const std::vector<Real> & intnl,
                                               const std::vector<Real> & pm,
                                               const RankFourTensor & E_inv,
                                               const std::vector<bool> & active,
                                               const std::vector<bool> & deactivated_due_to_ld,
                                               std::vector<std::vector<Real>> & jac)
{
  // see comments at the start of .h file

  mooseAssert(intnl.size() == _num_models,
              "Size of intnl is " << intnl.size() << " which is incorrect in calculateJacobian");
  mooseAssert(pm.size() == _num_surfaces,
              "Size of pm is " << pm.size() << " which is incorrect in calculateJacobian");
  mooseAssert(active.size() == _num_surfaces,
              "Size of active is " << active.size() << " which is incorrect in calculateJacobian");
  mooseAssert(deactivated_due_to_ld.size() == _num_surfaces,
              "Size of deactivated_due_to_ld is " << deactivated_due_to_ld.size()
                                                  << " which is incorrect in calculateJacobian");

  unsigned ind = 0;
  unsigned active_surface_ind = 0;

  std::vector<bool> active_surface(_num_surfaces); // active and not deactivated_due_to_ld
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    active_surface[surface] = (active[surface] && !deactivated_due_to_ld[surface]);
  unsigned num_active_surface = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (active_surface[surface])
      num_active_surface++;

  std::vector<bool> active_model(
      _num_models); // whether a model has surfaces that are active and not deactivated_due_to_ld
  for (unsigned model = 0; model < _num_models; ++model)
    active_model[model] = anyActiveSurfaces(model, active_surface);

  unsigned num_active_model = 0;
  for (unsigned model = 0; model < _num_models; ++model)
    if (active_model[model])
      num_active_model++;

  ind = 0;
  std::vector<unsigned int> active_model_index(_num_models);
  for (unsigned model = 0; model < _num_models; ++model)
    if (active_model[model])
      active_model_index[model] = ind++;
    else
      active_model_index[model] =
          _num_models + 1; // just a dummy, that will probably cause a crash if something goes wrong

  std::vector<RankTwoTensor> df_dstress;
  dyieldFunction_dstress(stress, intnl, active_surface, df_dstress);

  std::vector<Real> df_dintnl;
  dyieldFunction_dintnl(stress, intnl, active_surface, df_dintnl);

  std::vector<RankTwoTensor> r;
  flowPotential(stress, intnl, active, r);

  std::vector<RankFourTensor> dr_dstress;
  dflowPotential_dstress(stress, intnl, active, dr_dstress);

  std::vector<RankTwoTensor> dr_dintnl;
  dflowPotential_dintnl(stress, intnl, active, dr_dintnl);

  std::vector<Real> h;
  hardPotential(stress, intnl, active, h);

  std::vector<RankTwoTensor> dh_dstress;
  dhardPotential_dstress(stress, intnl, active, dh_dstress);

  std::vector<Real> dh_dintnl;
  dhardPotential_dintnl(stress, intnl, active, dh_dintnl);

  // d(epp)/dstress = sum_{active alpha} pm[alpha]*dr_dstress
  RankFourTensor depp_dstress;
  ind = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (active[surface]) // includes deactivated_due_to_ld
      depp_dstress += pm[surface] * dr_dstress[ind++];
  depp_dstress += E_inv;

  // d(epp)/dpm_{active_surface_index} = r_{active_surface_index}
  std::vector<RankTwoTensor> depp_dpm;
  depp_dpm.resize(num_active_surface);
  ind = 0;
  active_surface_ind = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    if (active[surface])
    {
      if (active_surface[surface]) // do not include the deactived_due_to_ld, since their pm are not
                                   // dofs in the NR
        depp_dpm[active_surface_ind++] = r[ind];
      ind++;
    }
  }

  // d(epp)/dintnl_{active_model_index} = sum(pm[asdf]*dr_dintnl[fdsa])
  std::vector<RankTwoTensor> depp_dintnl;
  depp_dintnl.assign(num_active_model, RankTwoTensor());
  ind = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    if (active[surface])
    {
      unsigned int model_num = modelNumber(surface);
      if (active_model[model_num]) // only include models with surfaces which are still active after
                                   // deactivated_due_to_ld
        depp_dintnl[active_model_index[model_num]] += pm[surface] * dr_dintnl[ind];
      ind++;
    }
  }

  // df_dstress has been calculated above
  // df_dpm is always zero
  // df_dintnl has been calculated above, but only the active_surface+active_model stuff needs to be
  // included in Jacobian: see below

  std::vector<RankTwoTensor> dic_dstress;
  dic_dstress.assign(num_active_model, RankTwoTensor());
  ind = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    if (active[surface])
    {
      unsigned int model_num = modelNumber(surface);
      if (active_model[model_num]) // only include ic for models with active_surface (ie, if model
                                   // only contains deactivated_due_to_ld don't include it)
        dic_dstress[active_model_index[model_num]] += pm[surface] * dh_dstress[ind];
      ind++;
    }
  }

  std::vector<std::vector<Real>> dic_dpm;
  dic_dpm.resize(num_active_model);
  ind = 0;
  active_surface_ind = 0;
  for (unsigned model = 0; model < num_active_model; ++model)
    dic_dpm[model].assign(num_active_surface, 0);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    if (active[surface])
    {
      if (active_surface[surface]) // only take derivs wrt active-but-not-deactivated_due_to_ld pm
      {
        unsigned int model_num = modelNumber(surface);
        // if (active_model[model_num]) // do not need this check as if the surface has
        // active_surface, the model must be deemed active!
        dic_dpm[active_model_index[model_num]][active_surface_ind] = h[ind];
        active_surface_ind++;
      }
      ind++;
    }
  }

  std::vector<std::vector<Real>> dic_dintnl;
  dic_dintnl.resize(num_active_model);
  for (unsigned model = 0; model < num_active_model; ++model)
  {
    dic_dintnl[model].assign(num_active_model, 0);
    dic_dintnl[model][model] = 1; // deriv wrt internal parameter
  }
  ind = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
  {
    if (active[surface])
    {
      unsigned int model_num = modelNumber(surface);
      if (active_model[model_num]) // only the models that contain surfaces that are still active
                                   // after deactivation_due_to_ld
        dic_dintnl[active_model_index[model_num]][active_model_index[model_num]] +=
            pm[surface] * dh_dintnl[ind];
      ind++;
    }
  }

  unsigned int dim = 3;
  unsigned int system_size =
      6 + num_active_surface + num_active_model; // "6" comes from symmeterizing epp
  jac.resize(system_size);
  for (unsigned i = 0; i < system_size; ++i)
    jac[i].assign(system_size, 0);

  unsigned int row_num = 0;
  unsigned int col_num = 0;
  for (unsigned i = 0; i < dim; ++i)
    for (unsigned j = 0; j <= i; ++j)
    {
      for (unsigned k = 0; k < dim; ++k)
        for (unsigned l = 0; l <= k; ++l)
          jac[col_num][row_num++] =
              depp_dstress(i, j, k, l) +
              (k != l ? depp_dstress(i, j, l, k)
                      : 0); // extra part is needed because i assume dstress(i, j) = dstress(j, i)
      for (unsigned surface = 0; surface < num_active_surface; ++surface)
        jac[col_num][row_num++] = depp_dpm[surface](i, j);
      for (unsigned a = 0; a < num_active_model; ++a)
        jac[col_num][row_num++] = depp_dintnl[a](i, j);
      row_num = 0;
      col_num++;
    }

  ind = 0;
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (active_surface[surface])
    {
      for (unsigned k = 0; k < dim; ++k)
        for (unsigned l = 0; l <= k; ++l)
          jac[col_num][row_num++] =
              df_dstress[ind](k, l) +
              (k != l ? df_dstress[ind](l, k)
                      : 0); // extra part is needed because i assume dstress(i, j) = dstress(j, i)
      for (unsigned beta = 0; beta < num_active_surface; ++beta)
        jac[col_num][row_num++] = 0; // df_dpm
      for (unsigned model = 0; model < _num_models; ++model)
        if (active_model[model]) // only use df_dintnl for models in active_model
        {
          if (modelNumber(surface) == model)
            jac[col_num][row_num++] = df_dintnl[ind];
          else
            jac[col_num][row_num++] = 0;
        }
      ind++;
      row_num = 0;
      col_num++;
    }

  for (unsigned a = 0; a < num_active_model; ++a)
  {
    for (unsigned k = 0; k < dim; ++k)
      for (unsigned l = 0; l <= k; ++l)
        jac[col_num][row_num++] =
            dic_dstress[a](k, l) +
            (k != l ? dic_dstress[a](l, k)
                    : 0); // extra part is needed because i assume dstress(i, j) = dstress(j, i)
    for (unsigned alpha = 0; alpha < num_active_surface; ++alpha)
      jac[col_num][row_num++] = dic_dpm[a][alpha];
    for (unsigned b = 0; b < num_active_model; ++b)
      jac[col_num][row_num++] = dic_dintnl[a][b];
    row_num = 0;
    col_num++;
  }

  mooseAssert(col_num == system_size, "Incorrect filling of cols in Jacobian");
}

void
MultiPlasticityLinearSystem::nrStep(const RankTwoTensor & stress,
                                    const std::vector<Real> & intnl_old,
                                    const std::vector<Real> & intnl,
                                    const std::vector<Real> & pm,
                                    const RankFourTensor & E_inv,
                                    const RankTwoTensor & delta_dp,
                                    RankTwoTensor & dstress,
                                    std::vector<Real> & dpm,
                                    std::vector<Real> & dintnl,
                                    const std::vector<bool> & active,
                                    std::vector<bool> & deactivated_due_to_ld)
{
  // Calculate RHS and Jacobian
  std::vector<Real> rhs;
  calculateRHS(stress, intnl_old, intnl, pm, delta_dp, rhs, active, true, deactivated_due_to_ld);

  std::vector<std::vector<Real>> jac;
  calculateJacobian(stress, intnl, pm, E_inv, active, deactivated_due_to_ld, jac);

  // prepare for LAPACKgesv_ routine provided by PETSc
  PetscBLASInt system_size = rhs.size();

  std::vector<double> a(system_size * system_size);
  // Fill in the a "matrix" by going down columns
  unsigned ind = 0;
  for (int col = 0; col < system_size; ++col)
    for (int row = 0; row < system_size; ++row)
      a[ind++] = jac[row][col];

  PetscBLASInt nrhs = 1;
  std::vector<PetscBLASInt> ipiv(system_size);
  PetscBLASInt info;
  LAPACKgesv_(&system_size, &nrhs, &a[0], &system_size, &ipiv[0], &rhs[0], &system_size, &info);

  if (info != 0)
    mooseError("In solving the linear system in a Newton-Raphson process, the PETSC LAPACK gsev "
               "routine returned with error code ",
               info);

  // Extract the results back to dstress, dpm and dintnl
  std::vector<bool> active_not_deact(_num_surfaces);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    active_not_deact[surface] = (active[surface] && !deactivated_due_to_ld[surface]);

  unsigned int dim = 3;
  ind = 0;

  for (unsigned i = 0; i < dim; ++i)
    for (unsigned j = 0; j <= i; ++j)
      dstress(i, j) = dstress(j, i) = rhs[ind++];
  dpm.assign(_num_surfaces, 0);
  for (unsigned surface = 0; surface < _num_surfaces; ++surface)
    if (active_not_deact[surface])
      dpm[surface] = rhs[ind++];
  dintnl.assign(_num_models, 0);
  for (unsigned model = 0; model < _num_models; ++model)
    if (anyActiveSurfaces(model, active_not_deact))
      dintnl[model] = rhs[ind++];

  mooseAssert(static_cast<int>(ind) == system_size,
              "Incorrect extracting of changes from NR solution in nrStep");
}
