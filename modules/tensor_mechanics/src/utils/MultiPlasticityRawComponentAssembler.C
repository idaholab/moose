/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "MultiPlasticityRawComponentAssembler.h"

template<>
InputParameters validParams<MultiPlasticityRawComponentAssembler>()
{
  InputParameters params = emptyInputParameters();
  MooseEnum specialIC("none rock joint", "none");
  params.addParam<MooseEnum>("specialIC", specialIC, "For certain combinations of plastic models, the set of active constraints can be initialized optimally.  'none': no special initialization is performed.  For all other choices, the plastic_models must be chosen to have the following types.  'rock': 'TensileMulti MohrCoulombMulti'.  'joint': 'WeakPlaneTensile WeakPlaneShear'.");
  params.addRequiredParam<std::vector<UserObjectName> >("plastic_models", "List of names of user objects that define the plastic models that could be active for this material.");
  params.addClassDescription("RawComponentAssembler class to calculate yield functions, etc, used in multi-surface finite-strain plasticity");
  return params;
}

MultiPlasticityRawComponentAssembler::MultiPlasticityRawComponentAssembler(const std::string & /*name*/,
                                         InputParameters parameters):
  _num_models(parameters.get<std::vector<UserObjectName> >("plastic_models").size()),
  _num_surfaces(0),
  _specialIC(parameters.get<MooseEnum>("specialIC"))
{
  _f.resize(_num_models);
  UserObjectInterface uoi(parameters); // this comes via TensorMechanicsPlasticModel.  i haven't derived this class from UserObjectInterface because i'm worried about diamond inheritance when using this in FiniteStrainMultiPlasticity
  for (unsigned model = 0 ; model < _num_models ; ++model)
    _f[model] = &uoi.getUserObjectByName<TensorMechanicsPlasticModel>(parameters.get<std::vector<UserObjectName> >("plastic_models")[model]);

  for (unsigned model = 0 ; model < _num_models ; ++model)
    _num_surfaces += _f[model]->numberSurfaces();

  _model_given_surface.resize(_num_surfaces);
  _model_surface_given_surface.resize(_num_surfaces);
  unsigned int surface = 0;
  for (unsigned model = 0 ; model < _num_models ; ++model)
    for (unsigned model_surface = 0 ; model_surface < _f[model]->numberSurfaces() ; ++model_surface)
    {
      _model_given_surface[surface] = model;
      _model_surface_given_surface[surface] = model_surface;
      surface++;
    }

  _surfaces_given_model.resize(_num_models);
  surface = 0;
  for (unsigned model = 0 ; model < _num_models ; ++model)
  {
    _surfaces_given_model[model].resize(_f[model]->numberSurfaces());
    for (unsigned model_surface = 0 ; model_surface < _f[model]->numberSurfaces() ; ++model_surface)
      _surfaces_given_model[model][model_surface] = surface++;
  }

  // check the plastic_models for specialIC
  if (_specialIC == "rock")
  {
    if (_num_models != 2)
      mooseError("Choosing specialIC=rock, you must have plasticity models of type 'TensileMulti MohrCoulombMulti'\n");
    if (!(_f[0]->modelName().compare("TensileMulti") == 0 && _f[1]->modelName().compare("MohrCoulombMulti") == 0))
      mooseError("Choosing specialIC=rock, you must have plasticity models of type 'TensileMulti MohrCoulombMulti'\n");
  }
  if (_specialIC == "joint")
  {
    if (_num_models != 2)
      mooseError("Choosing specialIC=joint, you must have plasticity models of type 'WeakPlaneTensile WeakPlaneShear'\n");
    if (!(_f[0]->modelName().compare("WeakPlaneTensile") == 0 && _f[1]->modelName().compare("WeakPlaneShear") == 0))
      mooseError("Choosing specialIC=joint, you must have plasticity models of type 'WeakPlaneTensile WeakPlaneShear'\n");
  }
}


void
MultiPlasticityRawComponentAssembler::yieldFunction(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<Real> & f)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  f.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<Real> model_f;
  for (unsigned model = 0 ; model < _num_models ; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->yieldFunctionV(stress, intnl[model], model_f);
      for (active_surface = active_surfaces_of_model.begin() ; active_surface != active_surfaces_of_model.end() ; ++ active_surface)
        f.push_back(model_f[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dyieldFunction_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<RankTwoTensor> & df_dstress)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  df_dstress.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<RankTwoTensor> model_df_dstress;
  for (unsigned model = 0 ; model < _num_models ; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dyieldFunction_dstressV(stress, intnl[model], model_df_dstress);
      for (active_surface = active_surfaces_of_model.begin() ; active_surface != active_surfaces_of_model.end() ; ++ active_surface)
        df_dstress.push_back(model_df_dstress[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dyieldFunction_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<Real> & df_dintnl)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  df_dintnl.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<Real> model_df_dintnl;
  for (unsigned model = 0 ; model < _num_models ; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dyieldFunction_dintnlV(stress, intnl[model], model_df_dintnl);
      for (active_surface = active_surfaces_of_model.begin() ; active_surface != active_surfaces_of_model.end() ; ++ active_surface)
        df_dintnl.push_back(model_df_dintnl[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::flowPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<RankTwoTensor> & r)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  r.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<RankTwoTensor> model_r;
  for (unsigned model = 0 ; model < _num_models ; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->flowPotentialV(stress, intnl[model], model_r);
      for (active_surface = active_surfaces_of_model.begin() ; active_surface != active_surfaces_of_model.end() ; ++ active_surface)
        r.push_back(model_r[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dflowPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<RankFourTensor> & dr_dstress)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  dr_dstress.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<RankFourTensor> model_dr_dstress;
  for (unsigned model = 0 ; model < _num_models ; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dflowPotential_dstressV(stress, intnl[model], model_dr_dstress);
      for (active_surface = active_surfaces_of_model.begin() ; active_surface != active_surfaces_of_model.end() ; ++ active_surface)
        dr_dstress.push_back(model_dr_dstress[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dflowPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<RankTwoTensor> & dr_dintnl)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  dr_dintnl.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<RankTwoTensor> model_dr_dintnl;
  for (unsigned model = 0 ; model < _num_models ; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dflowPotential_dintnlV(stress, intnl[model], model_dr_dintnl);
      for (active_surface = active_surfaces_of_model.begin() ; active_surface != active_surfaces_of_model.end() ; ++ active_surface)
        dr_dintnl.push_back(model_dr_dintnl[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::hardPotential(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<Real> & h)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  h.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<Real> model_h;
  for (unsigned model = 0 ; model < _num_models ; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->hardPotentialV(stress, intnl[model], model_h);
      for (active_surface = active_surfaces_of_model.begin() ; active_surface != active_surfaces_of_model.end() ; ++ active_surface)
        h.push_back(model_h[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dhardPotential_dstress(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<RankTwoTensor> & dh_dstress)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  dh_dstress.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<RankTwoTensor> model_dh_dstress;
  for (unsigned model = 0 ; model < _num_models ; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dhardPotential_dstressV(stress, intnl[model], model_dh_dstress);
      for (active_surface = active_surfaces_of_model.begin() ; active_surface != active_surfaces_of_model.end() ; ++ active_surface)
        dh_dstress.push_back(model_dh_dstress[*active_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::dhardPotential_dintnl(const RankTwoTensor & stress, const std::vector<Real> & intnl, const std::vector<bool> & active, std::vector<Real> & dh_dintnl)
{
  mooseAssert(intnl.size() == _num_models, "Incorrect size of internal parameters");
  mooseAssert(active.size() == _num_surfaces, "Incorrect size of active");

  dh_dintnl.resize(0);
  std::vector<unsigned int> active_surfaces_of_model;
  std::vector<unsigned int>::iterator active_surface;
  std::vector<Real> model_dh_dintnl;
  for (unsigned model = 0 ; model < _num_models ; ++model)
  {
    activeModelSurfaces(model, active, active_surfaces_of_model);
    if (active_surfaces_of_model.size() > 0)
    {
      _f[model]->dhardPotential_dintnlV(stress, intnl[model], model_dh_dintnl);
      for (active_surface = active_surfaces_of_model.begin() ; active_surface != active_surfaces_of_model.end() ; ++ active_surface)
        dh_dintnl.push_back(model_dh_dintnl[*active_surface]);
    }
  }
}


void
MultiPlasticityRawComponentAssembler::buildActiveConstraints(const std::vector<Real> & f, const RankTwoTensor & stress, const std::vector<Real> & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act)
{
  mooseAssert(f.size() == _num_surfaces, "buildActiveConstraints called with f.size = " << f.size() << " while there are " << _num_surfaces << " surfaces");
  mooseAssert(intnl.size() == _num_models, "buildActiveConstraints called with intnl.size = " << intnl.size() << " while there are " << _num_models << " models");

  if (_specialIC == "rock")
    buildActiveConstraintsRock(f, stress, intnl, Eijkl, act);
  else if (_specialIC == "joint")
    buildActiveConstraintsJoint(f, stress, intnl, Eijkl, act);
  else // no specialIC
  {
    act.resize(0);
    unsigned ind = 0;
    for (unsigned model = 0 ; model < _num_models ; ++model)
    {
      std::vector<Real> model_f(0);
      for (unsigned model_surface = 0 ; model_surface < _f[model]->numberSurfaces() ; ++model_surface)
        model_f.push_back(f[ind++]);
      std::vector<bool> model_act;
      RankTwoTensor returned_stress;
      _f[model]->activeConstraints(model_f, stress, intnl[model], Eijkl, model_act, returned_stress);
      for (unsigned model_surface = 0 ; model_surface < _f[model]->numberSurfaces() ; ++model_surface)
        act.push_back(model_act[model_surface]);
    }
  }
}

void
MultiPlasticityRawComponentAssembler::buildActiveConstraintsJoint(const std::vector<Real> & f, const RankTwoTensor & stress, const std::vector<Real> & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act)
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
MultiPlasticityRawComponentAssembler::buildActiveConstraintsRock(const std::vector<Real> & f, const RankTwoTensor & stress, const std::vector<Real> & intnl, const RankFourTensor & Eijkl, std::vector<bool> & act)
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
  if (f_single[0] <= _f[1]->_f_tol && f_single[1] <= _f[1]->_f_tol && f_single[2] <= _f[1]->_f_tol && f_single[3] <= _f[1]->_f_tol && f_single[4] <= _f[1]->_f_tol && f_single[5] <= _f[1]->_f_tol)
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
  if (active_tensile[0] == false && active_tensile[1] == true && active_tensile[2] == true && active_MC[0] == true && active_MC[1] == true && active_MC[2] == false && active_MC[3] == true && active_MC[4] == false && active_MC[5] == false)
  {
    act[1] = act[2] = act[6] = true;
    act[4] = true;
    return;
  }
  if (active_tensile[0] == false && active_tensile[1] == true && active_tensile[2] == true && active_MC[0] == false && active_MC[1] == true && active_MC[2] == false && active_MC[3] == true && active_MC[4] == false && active_MC[5] == true)
  {
    act[1] = act[2] = act[6] = true;  // i don't think act[4] is necessary, is it?!
    return;
  }

  // tensile = edge, MC=edge (two possibilities)
  if (active_tensile[0] == false && active_tensile[1] == true && active_tensile[2] == true && active_MC[0] == false && active_MC[1] == true && active_MC[2] == false && active_MC[3] == true && active_MC[4] == false && active_MC[5] == false)
  {
    act[1] = act[2] = act[4] = act[6] = true;
    return;
  }
  if (active_tensile[0] == false && active_tensile[1] == true && active_tensile[2] == true && active_MC[0] == false && active_MC[1] == false && active_MC[2] == false && active_MC[3] == true && active_MC[4] == false && active_MC[5] == true)
  {
    act[1] = act[2] = act[4] = act[6] = true;
    return;
  }

  // tensile = edge, MC=face
  if (active_tensile[0] == false && active_tensile[1] == true && active_tensile[2] == true && active_MC[0] == false && active_MC[1] == false && active_MC[2] == false && active_MC[3] == true && active_MC[4] == false && active_MC[5] == false)
  {
    act[1] = act[2] = act[6] = true;
    return;
  }

  // tensile = face, MC=tip (two possibilities)
  if (active_tensile[0] == false && active_tensile[1] == false && active_tensile[2] == true && active_MC[0] == true && active_MC[1] == true && active_MC[2] == false && active_MC[3] == true && active_MC[4] == false && active_MC[5] == false)
  {
    act[2] = act[6] = true;
    act[4] = true;
    act[8] = true;
    return;
  }
  if (active_tensile[0] == false && active_tensile[1] == false && active_tensile[2] == true && active_MC[0] == false && active_MC[1] == true && active_MC[2] == false && active_MC[3] == true && active_MC[4] == false && active_MC[5] == true)
  {
    act[2] = act[6] = true;
    act[8] = true;
    return;
  }


  // tensile = face, MC=face
  if (active_tensile[0] == false && active_tensile[1] == false && active_tensile[2] == true && active_MC[0] == false && active_MC[1] == false && active_MC[2] == false && active_MC[3] == true && active_MC[4] == false && active_MC[5] == false)
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



unsigned int
MultiPlasticityRawComponentAssembler::modelNumber(unsigned int surface)
{
  return _model_given_surface[surface];
}

bool
MultiPlasticityRawComponentAssembler::anyActiveSurfaces(int model, const std::vector<bool> & active)
{
  for (unsigned model_surface = 0 ; model_surface < _f[model]->numberSurfaces() ; ++model_surface)
    if (active[_surfaces_given_model[model][model_surface]])
      return true;
  return false;
}

void
MultiPlasticityRawComponentAssembler::activeSurfaces(int model, const std::vector<bool> & active, std::vector<unsigned int> & active_surfaces)
{
  active_surfaces.resize(0);
  for (unsigned model_surface = 0 ; model_surface < _f[model]->numberSurfaces() ; ++model_surface)
    if (active[_surfaces_given_model[model][model_surface]])
      active_surfaces.push_back(_surfaces_given_model[model][model_surface]);
}

void
MultiPlasticityRawComponentAssembler::activeModelSurfaces(int model, const std::vector<bool> & active, std::vector<unsigned int> & active_surfaces_of_model)
{
  active_surfaces_of_model.resize(0);
  for (unsigned model_surface = 0 ; model_surface < _f[model]->numberSurfaces() ; ++model_surface)
    if (active[_surfaces_given_model[model][model_surface]])
      active_surfaces_of_model.push_back(model_surface);
}
