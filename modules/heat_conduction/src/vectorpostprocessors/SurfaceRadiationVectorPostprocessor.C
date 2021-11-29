//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceRadiationVectorPostprocessor.h"
#include "GrayLambertSurfaceRadiationBase.h"
#include <limits>
#include "libmesh/utility.h"

registerMooseObject("HeatConductionApp", SurfaceRadiationVectorPostprocessor);

InputParameters
SurfaceRadiationVectorPostprocessor::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "VectorPostprocessor for accessing information stored in surface radiation user object");
  params.addRequiredParam<UserObjectName>("surface_radiation_object_name",
                                          "Name of the GrayLambertSurfaceRadiationBase UO");
  MultiMooseEnum information_type("temperature=0 heat_flux_density=1 radiosity=2 emissivity=3",
                                  "temperature");
  params.addParam<MultiMooseEnum>(
      "information",
      information_type,
      "The type of information to obtain from surface radiation user object");
  return params;
}

SurfaceRadiationVectorPostprocessor::SurfaceRadiationVectorPostprocessor(
    const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _glsr_uo(getUserObject<GrayLambertSurfaceRadiationBase>("surface_radiation_object_name")),
    _information_types(getParam<MultiMooseEnum>("information")),
    _n_data(_information_types.size()),
    _data(_n_data),
    _surface_ids(declareVector("subdomain_id"))
{
  for (unsigned int j = 0; j < _n_data; ++j)
    _data[j] = &declareVector(_information_types[j]);
}

void
SurfaceRadiationVectorPostprocessor::initialize()
{
  std::set<BoundaryID> bids = _glsr_uo.getSurfaceIDs();
  _surface_ids.resize(bids.size());
  for (unsigned int j = 0; j < _n_data; ++j)
    _data[j]->resize(bids.size());
  unsigned int j = 0;
  for (auto & bid : bids)
  {
    _surface_ids[j] = bid;
    ++j;
  }
}

void
SurfaceRadiationVectorPostprocessor::execute()
{
  for (unsigned int i = 0; i < _n_data; ++i)
  {
    switch (_information_types.get(i))
    {
      case 0:
        for (unsigned int j = 0; j < _surface_ids.size(); ++j)
          (*_data[i])[j] = _glsr_uo.getSurfaceTemperature(_surface_ids[j]);
        break;
      case 1:
        for (unsigned int j = 0; j < _surface_ids.size(); ++j)
          (*_data[i])[j] = _glsr_uo.getSurfaceHeatFluxDensity(_surface_ids[j]);
        break;
      case 2:
        for (unsigned int j = 0; j < _surface_ids.size(); ++j)
          (*_data[i])[j] = _glsr_uo.getSurfaceRadiosity(_surface_ids[j]);
        break;
      case 3:
        for (unsigned int j = 0; j < _surface_ids.size(); ++j)
          (*_data[i])[j] = _glsr_uo.getSurfaceEmissivity(_surface_ids[j]);
        break;
      default:
        mooseError("Unrecognized information type. This should never happen");
        break;
    }
  }
}
