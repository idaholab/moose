//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiPlasticityRawComponentAssembler.h"
#include "RankFourTensor.h"

InputParameters
MultiPlasticityRawComponentAssembler::validParams()
{
  InputParameters params = emptyInputParameters();
  MooseEnum specialIC("none rock joint", "none");
  params.addParam<MooseEnum>("specialIC",
                             specialIC,
                             "For certain combinations of plastic models, the set of active "
                             "constraints can be initialized optimally.  'none': no special "
                             "initialization is performed.  For all other choices, the "
                             "plastic_models must be chosen to have the following types.  'rock': "
                             "'TensileMulti MohrCoulombMulti'.  'joint': 'WeakPlaneTensile "
                             "WeakPlaneShear'.");
  params.addParam<std::vector<UserObjectName>>(
      "plastic_models",
      "List of names of user objects that define the plastic models that could "
      "be active for this material.  If no plastic_models are provided, only "
      "elasticity will be used.");
  params.addClassDescription("RawComponentAssembler class to calculate yield functions, etc, used "
                             "in multi-surface finite-strain plasticity");
  return params;
}

MultiPlasticityRawComponentAssembler::MultiPlasticityRawComponentAssembler(
    const MooseObject * moose_object)
  : UserObjectInterface(moose_object),
    _params(moose_object->parameters()),
    _num_models(_params.get<std::vector<UserObjectName>>("plastic_models").size()),
    _num_surfaces(0),
    _specialIC(_params.get<MooseEnum>("specialIC"))
{
  _f.resize(_num_models);
  for (unsigned model = 0; model < _num_models; ++model)
    _f[model] = &getUserObjectByName<TensorMechanicsPlasticModel>(
        _params.get<std::vector<UserObjectName>>("plastic_models")[model]);

  for (unsigned model = 0; model < _num_models; ++model)
    _num_surfaces += _f[model]->numberSurfaces();

  _model_given_surface.resize(_num_surfaces);
  _model_surface_given_surface.resize(_num_surfaces);
  unsigned int surface = 0;
  for (unsigned model = 0; model < _num_models; ++model)
    for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces(); ++model_surface)
    {
      _model_given_surface[surface] = model;
      _model_surface_given_surface[surface] = model_surface;
      surface++;
    }

  _surfaces_given_model.resize(_num_models);
  surface = 0;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    _surfaces_given_model[model].resize(_f[model]->numberSurfaces());
    for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces(); ++model_surface)
      _surfaces_given_model[model][model_surface] = surface++;
  }

  // check the plastic_models for specialIC
  if (_specialIC == "rock")
  {
    if (_num_models != 2)
      mooseError("Choosing specialIC=rock, you must have plasticity models of type 'TensileMulti "
                 "MohrCoulombMulti'\n");
    if (!(_f[0]->modelName().compare("TensileMulti") == 0 &&
          _f[1]->modelName().compare("MohrCoulombMulti") == 0))
      mooseError("Choosing specialIC=rock, you must have plasticity models of type 'TensileMulti "
                 "MohrCoulombMulti'\n");
  }
  if (_specialIC == "joint")
  {
    if (_num_models != 2)
      mooseError("Choosing specialIC=joint, you must have plasticity models of type "
                 "'WeakPlaneTensile WeakPlaneShear'\n");
    if (!(_f[0]->modelName().compare("WeakPlaneTensile") == 0 &&
          _f[1]->modelName().compare("WeakPlaneShear") == 0))
      mooseError("Choosing specialIC=joint, you must have plasticity models of type "
                 "'WeakPlaneTensile WeakPlaneShear'\n");
  }
}

void
MultiPlasticityRawComponentAssembler::yieldFunction(const RankTwoTensor & stress,
                                                    const std::vector<Real> & intnl,
                                                    const std::vector<bool> & active,
                                                    std::vector<Real> & f)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  f.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<Real> model_f;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->yieldFunctionV(stress, intnl[model], model_f);
      for (active_surface = active_surfaces_of_model.begin();
           active_surface != active_surfaces_of_model.end();
           ++active_surface)
        f.push_back(model_f[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dyieldFunction_dstress(
    const RankTwoTensor & stress,
    const std::vector<Real> & intnl,
    const std::vector<bool> & active,
    std::vector<RankTwoTensor> & df_dstress)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  df_dstress.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<RankTwoTensor> model_df_dstress;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dyieldFunction_dstressV(stress, intnl[model], model_df_dstress);
      for (active_surface = active_surfaces_of_model.begin();
           active_surface != active_surfaces_of_model.end();
           ++active_surface)
        df_dstress.push_back(model_df_dstress[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dyieldFunction_dintnl(const RankTwoTensor & stress,
                                                            const std::vector<Real> & intnl,
                                                            const std::vector<bool> & active,
                                                            std::vector<Real> & df_dintnl)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  df_dintnl.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<Real> model_df_dintnl;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dyieldFunction_dintnlV(stress, intnl[model], model_df_dintnl);
      for (active_surface = active_surfaces_of_model.begin();
           active_surface != active_surfaces_of_model.end();
           ++active_surface)
        df_dintnl.push_back(model_df_dintnl[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::flowPotential(const RankTwoTensor & stress,
                                                    const std::vector<Real> & intnl,
                                                    const std::vector<bool> & active,
                                                    std::vector<RankTwoTensor> & r)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  r.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<RankTwoTensor> model_r;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->flowPotentialV(stress, intnl[model], model_r);
      for (active_surface = active_surfaces_of_model.begin();
           active_surface != active_surfaces_of_model.end();
           ++active_surface)
        r.push_back(model_r[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dflowPotential_dstress(
    const RankTwoTensor & stress,
    const std::vector<Real> & intnl,
    const std::vector<bool> & active,
    std::vector<RankFourTensor> & dr_dstress)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  dr_dstress.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<RankFourTensor> model_dr_dstress;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dflowPotential_dstressV(stress, intnl[model], model_dr_dstress);
      for (active_surface = active_surfaces_of_model.begin();
           active_surface != active_surfaces_of_model.end();
           ++active_surface)
        dr_dstress.push_back(model_dr_dstress[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dflowPotential_dintnl(const RankTwoTensor & stress,
                                                            const std::vector<Real> & intnl,
                                                            const std::vector<bool> & active,
                                                            std::vector<RankTwoTensor> & dr_dintnl)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  dr_dintnl.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<RankTwoTensor> model_dr_dintnl;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dflowPotential_dintnlV(stress, intnl[model], model_dr_dintnl);
      for (active_surface = active_surfaces_of_model.begin();
           active_surface != active_surfaces_of_model.end();
           ++active_surface)
        dr_dintnl.push_back(model_dr_dintnl[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::hardPotential(const RankTwoTensor & stress,
                                                    const std::vector<Real> & intnl,
                                                    const std::vector<bool> & active,
                                                    std::vector<Real> & h)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  h.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<Real> model_h;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->hardPotentialV(stress, intnl[model], model_h);
      for (active_surface = active_surfaces_of_model.begin();
           active_surface != active_surfaces_of_model.end();
           ++active_surface)
        h.push_back(model_h[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dhardPotential_dstress(
    const RankTwoTensor & stress,
    const std::vector<Real> & intnl,
    const std::vector<bool> & active,
    std::vector<RankTwoTensor> & dh_dstress)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  dh_dstress.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<RankTwoTensor> model_dh_dstress;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dhardPotential_dstressV(stress, intnl[model], model_dh_dstress);
      for (active_surface = active_surfaces_of_model.begin();
           active_surface != active_surfaces_of_model.end();
           ++active_surface)
        dh_dstress.push_back(model_dh_dstress[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dhardPotential_dintnl(const RankTwoTensor & stress,
                                                            const std::vector<Real> & intnl,
                                                            const std::vector<bool> & active,
                                                            std::vector<Real> & dh_dintnl)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  dh_dintnl.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<Real> model_dh_dintnl;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dhardPotential_dintnlV(stress, intnl[model], model_dh_dintnl);
      for (active_surface = active_surfaces_of_model.begin();
           active_surface != active_surfaces_of_model.end();
           ++active_surface)
        dh_dintnl.push_back(model_dh_dintnl[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::buildActiveConstraints(const std::vector<Real> & f,
                                                             const RankTwoTensor & stress,
                                                             const std::vector<Real> & intnl,
                                                             const RankFourTensor & Eijkl,
                                                             std::vector<bool> & act)
{
  mooseAssert(f.size() == _num_surfaces,
              "buildActiveConstraints called with f.size = " << f.size() << " while there are "
                                                             << _num_surfaces << " surfaces");
  mooseAssert(intnl.size() == _num_models,
              "buildActiveConstraints called with intnl.size = "
                  << intnl.size() << " while there are " << _num_models << " models");

  if (_specialIC == "rock")
    buildActiveConstraintsRock(f, stress, intnl, Eijkl, act);
  else if (_specialIC == "joint")
    buildActiveConstraintsJoint(f, stress, intnl, Eijkl, act);
  else // no specialIC
  {
    act.resize(0);
    unsigned ind = 0;
    for (unsigned model = 0; model < _num_models; ++model)
    {
      std::vector<Real> model_f(0);
      for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces(); ++model_surface)
        model_f.push_back(f[ind++]);
      std::vector<bool> model_act;
      RankTwoTensor returned_stress;
      _f[model]->activeConstraints(
          model_f, stress, intnl[model], Eijkl, model_act, returned_stress);
      for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces(); ++model_surface)
        act.push_back(model_act[model_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::buildActiveConstraintsJoint(const std::vector<Real> & f,
                                                                  const RankTwoTensor & stress,
                                                                  const std::vector<Real> & intnl,
                                                                  const RankFourTensor & Eijkl,
                                                                  std::vector<bool> & act)
{
  act.assign(2, false);

  RankTwoTensor returned_stress;
  std::vector<bool> active_tensile;
  std::vector<bool> active_shear;
  std::vector<Real> f_single;

  // first try tensile alone
  f_single.assign(1, 0);
  f_single[0] = f[0];
  _f[0]->activeConstraints(f_single, stress, intnl[0], Eijkl, active_tensile, returned_stress);
  _f[1]->yieldFunctionV(returned_stress, intnl[1], f_single);
  if (f_single[0] <= _f[1]->_f_tol)
  {
    act[0] = active_tensile[0];
    return;
  }

  // next try shear alone
  f_single.assign(1, 0);
  f_single[0] = f[1];
  _f[1]->activeConstraints(f_single, stress, intnl[1], Eijkl, active_shear, returned_stress);
  _f[0]->yieldFunctionV(returned_stress, intnl[0], f_single);
  if (f_single[0] <= _f[0]->_f_tol)
  {
    act[1] = active_shear[0];
    return;
  }

  // must be mixed
  act[0] = act[1] = true;
  return;
}

void
MultiPlasticityRawComponentAssembler::buildActiveConstraintsRock(const std::vector<Real> & f,
                                                                 const RankTwoTensor & stress,
                                                                 const std::vector<Real> & intnl,
                                                                 const RankFourTensor & Eijkl,
                                                                 std::vector<bool> & act)
{
  act.assign(9, false);

  RankTwoTensor returned_stress;
  std::vector<bool> active_tensile;
  std::vector<bool> active_MC;
  std::vector<Real> f_single;

  // first try tensile alone
  f_single.assign(3, 0);
  f_single[0] = f[0];
  f_single[1] = f[1];
  f_single[2] = f[2];
  _f[0]->activeConstraints(f_single, stress, intnl[0], Eijkl, active_tensile, returned_stress);
  _f[1]->yieldFunctionV(returned_stress, intnl[1], f_single);
  if (f_single[0] <= _f[1]->_f_tol && f_single[1] <= _f[1]->_f_tol &&
      f_single[2] <= _f[1]->_f_tol && f_single[3] <= _f[1]->_f_tol &&
      f_single[4] <= _f[1]->_f_tol && f_single[5] <= _f[1]->_f_tol)
  {
    act[0] = active_tensile[0];
    act[1] = active_tensile[1];
    act[2] = active_tensile[2];
    return;
  }

  // next try MC alone
  f_single.assign(6, 0);
  f_single[0] = f[3];
  f_single[1] = f[4];
  f_single[2] = f[5];
  f_single[3] = f[6];
  f_single[4] = f[7];
  f_single[5] = f[8];
  _f[1]->activeConstraints(f_single, stress, intnl[1], Eijkl, active_MC, returned_stress);
  _f[0]->yieldFunctionV(returned_stress, intnl[0], f_single);
  if (f_single[0] <= _f[0]->_f_tol && f_single[1] <= _f[0]->_f_tol && f_single[2] <= _f[0]->_f_tol)
  {
    act[3] = active_MC[0];
    act[4] = active_MC[1];
    act[5] = active_MC[2];
    act[6] = active_MC[3];
    act[7] = active_MC[4];
    act[8] = active_MC[5];
    return;
  }

  // must be a mix.
  // The possibilities are enumerated below.

  // tensile=edge, MC=tip (two possibilities)
  if (active_tensile[0] == false && active_tensile[1] == true && active_tensile[2] == true &&
      active_MC[0] == true && active_MC[1] == true && active_MC[2] == false &&
      active_MC[3] == true && active_MC[4] == false && active_MC[5] == false)
  {
    act[1] = act[2] = act[6] = true;
    act[4] = true;
    return;
  }
  if (active_tensile[0] == false && active_tensile[1] == true && active_tensile[2] == true &&
      active_MC[0] == false && active_MC[1] == true && active_MC[2] == false &&
      active_MC[3] == true && active_MC[4] == false && active_MC[5] == true)
  {
    act[1] = act[2] = act[6] = true; // i don't think act[4] is necessary, is it?!
    return;
  }

  // tensile = edge, MC=edge (two possibilities)
  if (active_tensile[0] == false && active_tensile[1] == true && active_tensile[2] == true &&
      active_MC[0] == false && active_MC[1] == true && active_MC[2] == false &&
      active_MC[3] == true && active_MC[4] == false && active_MC[5] == false)
  {
    act[1] = act[2] = act[4] = act[6] = true;
    return;
  }
  if (active_tensile[0] == false && active_tensile[1] == true && active_tensile[2] == true &&
      active_MC[0] == false && active_MC[1] == false && active_MC[2] == false &&
      active_MC[3] == true && active_MC[4] == false && active_MC[5] == true)
  {
    act[1] = act[2] = act[4] = act[6] = true;
    return;
  }

  // tensile = edge, MC=face
  if (active_tensile[0] == false && active_tensile[1] == true && active_tensile[2] == true &&
      active_MC[0] == false && active_MC[1] == false && active_MC[2] == false &&
      active_MC[3] == true && active_MC[4] == false && active_MC[5] == false)
  {
    act[1] = act[2] = act[6] = true;
    return;
  }

  // tensile = face, MC=tip (two possibilities)
  if (active_tensile[0] == false && active_tensile[1] == false && active_tensile[2] == true &&
      active_MC[0] == true && active_MC[1] == true && active_MC[2] == false &&
      active_MC[3] == true && active_MC[4] == false && active_MC[5] == false)
  {
    act[2] = act[6] = true;
    act[4] = true;
    act[8] = true;
    return;
  }
  if (active_tensile[0] == false && active_tensile[1] == false && active_tensile[2] == true &&
      active_MC[0] == false && active_MC[1] == true && active_MC[2] == false &&
      active_MC[3] == true && active_MC[4] == false && active_MC[5] == true)
  {
    act[2] = act[6] = true;
    act[8] = true;
    return;
  }

  // tensile = face, MC=face
  if (active_tensile[0] == false && active_tensile[1] == false && active_tensile[2] == true &&
      active_MC[0] == false && active_MC[1] == false && active_MC[2] == false &&
      active_MC[3] == true && active_MC[4] == false && active_MC[5] == false)
  {
    act[1] = act[2] = act[6] = true;
    return;
  }

  // tensile = face, MC=edge (two possibilites).
  act[2] = true; // tensile face
  act[3] = active_MC[0];
  act[4] = active_MC[1];
  act[5] = active_MC[2];
  act[6] = active_MC[3];
  act[7] = active_MC[4];
  act[8] = active_MC[5];
  return;
}

/**
 * Performs a returnMap for each plastic model.
 *
 * If all models actually signal "elastic" by
 * returning true from their returnMap, and
 * by returning model_plastically_active=0, then
 *   yf will contain the yield function values
 *   num_successful_plastic_returns will be zero
 *   intnl = intnl_old
 *   delta_dp will be unchanged from its input value
 *   stress will be set to trial_stress
 *   pm will be zero
 *   cumulative_pm will be unchanged
 *   return value will be true
 *   num_successful_plastic_returns = 0
 *
 * If only one model signals "plastically active"
 * by returning true from its returnMap,
 * and by returning model_plastically_active=1, then
 *   yf will contain the yield function values
 *   num_successful_plastic_returns will be one
 *   intnl will be set by the returnMap algorithm
 *   delta_dp will be set by the returnMap algorithm
 *   stress will be set by the returnMap algorithm
 *   pm will be nonzero for the single model, and zero for other models
 *   cumulative_pm will be updated
 *   return value will be true
 *   num_successful_plastic_returns = 1
 *
 * If >1 model signals "plastically active"
 * or if >=1 model's returnMap fails, then
 *   yf will contain the yield function values
 *   num_successful_plastic_returns will be set appropriately
 *   intnl = intnl_old
 *   delta_dp will be unchanged from its input value
 *   stress will be set to trial_stress
 *   pm will be zero
 *   cumulative_pm will be unchanged
 *   return value will be true if all returnMap functions returned true, otherwise it will be false
 *   num_successful_plastic_returns is set appropriately
 */
bool
MultiPlasticityRawComponentAssembler::returnMapAll(const RankTwoTensor & trial_stress,
                                                   const std::vector<Real> & intnl_old,
                                                   const RankFourTensor & E_ijkl,
                                                   Real ep_plastic_tolerance,
                                                   RankTwoTensor & stress,
                                                   std::vector<Real> & intnl,
                                                   std::vector<Real> & pm,
                                                   std::vector<Real> & cumulative_pm,
                                                   RankTwoTensor & delta_dp,
                                                   std::vector<Real> & yf,
                                                   unsigned & num_successful_plastic_returns,
                                                   unsigned & custom_model)
{
  mooseAssert(intnl_old.size() == _num_models,
              "returnMapAll: Incorrect size of internal parameters");
  mooseAssert(intnl.size() == _num_models, "returnMapAll: Incorrect size of internal parameters");
  mooseAssert(pm.size() == _num_surfaces, "returnMapAll: Incorrect size of pm");

  num_successful_plastic_returns = 0;
  yf.resize(0);
  pm.assign(_num_surfaces, 0.0);

  RankTwoTensor returned_stress; // each model will give a returned_stress.  if only one model is
                                 // plastically active, i set stress=returned_stress, so as to
                                 // record this returned value
  std::vector<Real> model_f;
  RankTwoTensor model_delta_dp;
  std::vector<Real> model_pm;
  bool trial_stress_inadmissible;
  bool successful_return = true;
  unsigned the_single_plastic_model = 0;
  bool using_custom_return_map = true;

  // run through all the plastic models, performing their
  // returnMap algorithms.
  // If one finds (trial_stress, intnl) inadmissible and
  // successfully returns, break from the loop to evaluate
  // all the others at that returned stress
  for (unsigned model = 0; model < _num_models; ++model)
  {
    if (using_custom_return_map)
    {
      model_pm.assign(_f[model]->numberSurfaces(), 0.0);
      bool model_returned = _f[model]->returnMap(trial_stress,
                                                 intnl_old[model],
                                                 E_ijkl,
                                                 ep_plastic_tolerance,
                                                 returned_stress,
                                                 intnl[model],
                                                 model_pm,
                                                 model_delta_dp,
                                                 model_f,
                                                 trial_stress_inadmissible);
      if (!trial_stress_inadmissible)
      {
        // in the elastic zone: record the yield-function values (returned_stress, intnl, model_pm
        // and model_delta_dp are undefined)
        for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces();
             ++model_surface)
          yf.push_back(model_f[model_surface]);
      }
      else if (trial_stress_inadmissible && !model_returned)
      {
        // in the plastic zone, and the customized returnMap failed
        // for some reason (or wasn't implemented).  The coder
        // should have correctly returned model_f(trial_stress, intnl_old)
        // so record them
        // (returned_stress, intnl, model_pm and model_delta_dp are undefined)
        for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces();
             ++model_surface)
          yf.push_back(model_f[model_surface]);
        // now there's almost zero point in using the custom
        // returnMap functions
        using_custom_return_map = false;
        successful_return = false;
      }
      else
      {
        // in the plastic zone, and the customized returnMap
        // succeeded.
        // record the first returned_stress and delta_dp if everything is going OK
        // as they could be the actual answer
        if (trial_stress_inadmissible)
          num_successful_plastic_returns++;
        the_single_plastic_model = model;
        stress = returned_stress;
        // note that i break here, and don't push_back
        // model_f to yf.  So now yf contains only the values of
        // model_f from previous models to the_single_plastic_model
        // also i don't set delta_dp = model_delta_dp yet, because
        // i might find problems later on
        // also, don't increment cumulative_pm for the same reason

        break;
      }
    }
    else
    {
      // not using custom returnMap functions because one
      // has already failed and that one said trial_stress
      // was inadmissible.  So now calculate the yield functions
      // at the trial stress
      _f[model]->yieldFunctionV(trial_stress, intnl_old[model], model_f);
      for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces(); ++model_surface)
        yf.push_back(model_f[model_surface]);
    }
  }

  if (num_successful_plastic_returns == 0)
  {
    // here either all the models were elastic (successful_return=true),
    // or some were plastic and either the customized returnMap failed
    // or wasn't implemented (successful_return=false).
    // In either case, have to set the following:
    stress = trial_stress;
    for (unsigned model = 0; model < _num_models; ++model)
      intnl[model] = intnl_old[model];
    return successful_return;
  }

  // Now we know that num_successful_plastic_returns == 1 and all the other
  // models (with model number < the_single_plastic_model) must have been
  // admissible at (trial_stress, intnl).  However, all models might
  // not be admissible at (trial_stress, intnl), so must check that
  std::vector<Real> yf_at_returned_stress(0);
  bool all_admissible = true;
  for (unsigned model = 0; model < _num_models; ++model)
  {
    if (model == the_single_plastic_model)
    {
      // no need to spend time calculating the yield function: we know its admissible
      for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces(); ++model_surface)
        yf_at_returned_stress.push_back(model_f[model_surface]);
      continue;
    }
    _f[model]->yieldFunctionV(stress, intnl_old[model], model_f);
    for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces(); ++model_surface)
    {
      if (model_f[model_surface] > _f[model]->_f_tol)
        // bummer, this model is not admissible at the returned_stress
        all_admissible = false;
      yf_at_returned_stress.push_back(model_f[model_surface]);
    }
    if (!all_admissible)
      // no point in continuing computing yield functions
      break;
  }

  if (!all_admissible)
  {
    // we tried using the returned value of stress predicted by
    // the_single_plastic_model, but it wasn't admissible according
    // to other plastic models.  We need to set:
    stress = trial_stress;
    for (unsigned model = 0; model < _num_models; ++model)
      intnl[model] = intnl_old[model];
    // and calculate the remainder of the yield functions at trial_stress
    for (unsigned model = the_single_plastic_model; model < _num_models; ++model)
    {
      _f[model]->yieldFunctionV(trial_stress, intnl[model], model_f);
      for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces(); ++model_surface)
        yf.push_back(model_f[model_surface]);
    }
    num_successful_plastic_returns = 0;
    return false;
  }

  // So the customized returnMap algorithm can provide a returned
  // (stress, intnl) configuration, and that is admissible according
  // to all plastic models
  yf.resize(0);
  for (unsigned surface = 0; surface < yf_at_returned_stress.size(); ++surface)
    yf.push_back(yf_at_returned_stress[surface]);
  delta_dp = model_delta_dp;
  for (unsigned model_surface = 0; model_surface < _f[the_single_plastic_model]->numberSurfaces();
       ++model_surface)
  {
    cumulative_pm[_surfaces_given_model[the_single_plastic_model][model_surface]] +=
        model_pm[model_surface];
    pm[_surfaces_given_model[the_single_plastic_model][model_surface]] = model_pm[model_surface];
  }
  custom_model = the_single_plastic_model;
  return true;
}

unsigned int
MultiPlasticityRawComponentAssembler::modelNumber(unsigned int surface)
{
  return _model_given_surface[surface];
}

bool
MultiPlasticityRawComponentAssembler::anyActiveSurfaces(int model, const std::vector<bool> & active)
{
  for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces(); ++model_surface)
    if (active[_surfaces_given_model[model][model_surface]])
      return true;
  return false;
}

void
MultiPlasticityRawComponentAssembler::activeSurfaces(int model,
                                                     const std::vector<bool> & active,
                                                     std::vector<unsigned int> & active_surfaces)
{
  active_surfaces.resize(0);
  for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces(); ++model_surface)
    if (active[_surfaces_given_model[model][model_surface]])
      active_surfaces.push_back(_surfaces_given_model[model][model_surface]);
}

void
MultiPlasticityRawComponentAssembler::activeModelSurfaces(
    int model,
    const std::vector<bool> & active,
    std::vector<unsigned int> & active_surfaces_of_model)
{
  active_surfaces_of_model.resize(0);
  for (unsigned model_surface = 0; model_surface < _f[model]->numberSurfaces(); ++model_surface)
    if (active[_surfaces_given_model[model][model_surface]])
      active_surfaces_of_model.push_back(model_surface);
}
