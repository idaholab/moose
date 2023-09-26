//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViewfactorVectorPostprocessor.h"
#include "GrayLambertSurfaceRadiationBase.h"

registerMooseObject("HeatConductionApp", ViewfactorVectorPostprocessor);

InputParameters
ViewfactorVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "VectorPostprocessor for accessing view factors from GrayLambertSurfaceRadiationBase UO");
  params.addRequiredParam<UserObjectName>("surface_radiation_object_name",
                                          "Name of the GrayLambertSurfaceRadiationBase UO");
  return params;
}

ViewfactorVectorPostprocessor::ViewfactorVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _glsr_uo(getUserObject<GrayLambertSurfaceRadiationBase>("surface_radiation_object_name")),
    _surface_ids(declareVector("subdomain_id"))
{
}

void
ViewfactorVectorPostprocessor::initialize()
{
  // setup of surface_id arrays
  std::set<BoundaryID> bids = _glsr_uo.getSurfaceIDs();
  unsigned int ns = bids.size();
  _surface_ids.resize(ns);
  unsigned int j = 0;
  for (auto & bid : bids)
  {
    _surface_ids[j] = bid;
    ++j;
  }

  // setup of view factors
  _vf.resize(ns);
  for (unsigned int j = 0; j < ns; ++j)
  {
    std::stringstream ss;
    ss << "vf_to_" << _surface_ids[j];
    _vf[j] = &declareVector(ss.str());
    _vf[j]->resize(ns);
  }
}

void
ViewfactorVectorPostprocessor::execute()
{
  for (unsigned int i = 0; i < _surface_ids.size(); ++i)
    for (unsigned int j = 0; j < _surface_ids.size(); ++j)
      (*_vf[j])[i] = _glsr_uo.getViewFactor(_surface_ids[i], _surface_ids[j]);
}
