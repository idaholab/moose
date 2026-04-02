//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViewFactorVectorPostprocessor.h"
#include "ViewFactorBase.h"
#include "GrayLambertSurfaceRadiationBase.h"

registerMooseObject("HeatTransferApp", ViewFactorVectorPostprocessor);
registerMooseObjectRenamed("HeatTransferApp",
                           ViewfactorVectorPostprocessor,
                           "08/30/2026 24:00",
                           ViewFactorVectorPostprocessor);

InputParameters
ViewFactorVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "VectorPostprocessor for accessing view factors from GrayLambertSurfaceRadiationBase UO");
  params.addParam<UserObjectName>("view_factor_object_name", "Name of the ViewFactorBase UO");
  // TODO: after deleting this parameter, make 'view_factor_object_name' required
  // and delete the associated error checks
  params.addDeprecatedParam<UserObjectName>("surface_radiation_object_name",
                                            "Name of the GrayLambertSurfaceRadiationBase UO",
                                            "Please use 'view_factor_object_name' instead.");
  return params;
}

ViewFactorVectorPostprocessor::ViewFactorVectorPostprocessor(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _view_factor_uo(isParamValid("view_factor_object_name")
                        ? &getUserObject<ViewFactorBase>("view_factor_object_name")
                        : nullptr),
    _glsr_uo(isParamValid("surface_radiation_object_name")
                 ? &getUserObject<GrayLambertSurfaceRadiationBase>("surface_radiation_object_name")
                 : nullptr),
    _surface_ids(declareVector("subdomain_id"))
{
  if (!isParamValid("surface_radiation_object_name") && !isParamValid("view_factor_object_name"))
    mooseError("The parameter 'view_factor_object_name' must be provided.");
  if (isParamValid("surface_radiation_object_name") && isParamValid("view_factor_object_name"))
    mooseError("The parameters 'surface_radiation_object_name' and 'view_factor_object_name' "
               "cannot both be provided. Please delete 'surface_radiation_object_name'.");
}

void
ViewFactorVectorPostprocessor::initialize()
{
  // setup of surface_id arrays
  std::set<BoundaryID> bids;
  if (_glsr_uo)
    bids = _glsr_uo->getSurfaceIDs();
  if (_view_factor_uo)
    bids = _view_factor_uo->boundaryIDs();

  unsigned int ns = bids.size();
  _surface_ids.resize(ns);
  unsigned int j = 0;
  for (auto & bid : bids)
  {
    _surface_ids[j] = bid;
    ++j;
  }

  // setup of view factors
  j = _vf.size();
  _vf.resize(ns);
  for (; j < ns; ++j)
  {
    std::stringstream ss;
    ss << "vf_to_" << _surface_ids[j];
    _vf[j] = &declareVector(ss.str());
    _vf[j]->resize(ns);
  }
}

void
ViewFactorVectorPostprocessor::execute()
{
  if (_view_factor_uo)
  {
    for (unsigned int i = 0; i < _surface_ids.size(); ++i)
      for (unsigned int j = 0; j < _surface_ids.size(); ++j)
        (*_vf[j])[i] = _view_factor_uo->getViewFactor(_surface_ids[i], _surface_ids[j]);
  }

  if (_glsr_uo)
  {
    for (unsigned int i = 0; i < _surface_ids.size(); ++i)
      for (unsigned int j = 0; j < _surface_ids.size(); ++j)
        (*_vf[j])[i] = _glsr_uo->getViewFactor(_surface_ids[i], _surface_ids[j]);
  }
}
