//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPlotPointFluxQuantity.h"
#include "PorousFlowPointFluxQuantity.h"

registerMooseObject("PorousFlowApp", PorousFlowPlotPointFluxQuantity);

InputParameters
PorousFlowPlotPointFluxQuantity::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>(
      "uo", "PorousFlowPointFluxQuantity user object name that holds the required information");
  params.addClassDescription(
      "Extracts the flux, and the coordinates, at each point of a line sink or Peaceman "
      "borehole from a PorousFlowPointFluxQuantity UserObject.  Note that the reported flux at "
      "each point is summed over the test functions of the element containing that point, so "
      "when nodal multiplicative factors (such as use_mobility) are active, it represents a "
      "test-function-weighted combination of those nodal values rather than a single nodal "
      "value.");
  return params;
}

PorousFlowPlotPointFluxQuantity::PorousFlowPlotPointFluxQuantity(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _point_fluxes(getUserObject<PorousFlowPointFluxQuantity>("uo")),
    _point_id(declareVector("point_id")),
    _x(declareVector("x")),
    _y(declareVector("y")),
    _z(declareVector("z")),
    _flux(declareVector("flux"))
{
}

void
PorousFlowPlotPointFluxQuantity::initialize()
{
  _point_id.clear();
  _x.clear();
  _y.clear();
  _z.clear();
  _flux.clear();
}

void
PorousFlowPlotPointFluxQuantity::execute()
{
  const std::vector<Real> & xs = _point_fluxes.getX();
  const std::vector<Real> & ys = _point_fluxes.getY();
  const std::vector<Real> & zs = _point_fluxes.getZ();
  const std::vector<Real> & fluxes = _point_fluxes.getFluxes();

  for (const auto i : index_range(fluxes))
  {
    _point_id.push_back(i);
    _x.push_back(xs[i]);
    _y.push_back(ys[i]);
    _z.push_back(zs[i]);
    _flux.push_back(fluxes[i]);
  }
}
