//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowPointFluxQuantity.h"

registerMooseObject("PorousFlowApp", PorousFlowPointFluxQuantity);

InputParameters
PorousFlowPointFluxQuantity::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Records the flux at each Dirac point of a PorousFlow line sink (such as "
      "PorousFlowPeacemanBorehole or PorousFlowPolyLineSink).  Use a "
      "PorousFlowPlotPointFluxQuantity VectorPostprocessor to output the recorded values.");
  return params;
}

PorousFlowPointFluxQuantity::PorousFlowPointFluxQuantity(const InputParameters & parameters)
  : GeneralUserObject(parameters)
{
}

void
PorousFlowPointFluxQuantity::zero(const std::vector<Real> & xs,
                                  const std::vector<Real> & ys,
                                  const std::vector<Real> & zs)
{
  _fluxes.assign(xs.size(), 0.0);
  _xs = xs;
  _ys = ys;
  _zs = zs;
}

void
PorousFlowPointFluxQuantity::add(std::size_t i, Real contrib)
{
  _fluxes[i] += contrib;
}

void
PorousFlowPointFluxQuantity::initialize()
{
}

void
PorousFlowPointFluxQuantity::execute()
{
}

void
PorousFlowPointFluxQuantity::finalize()
{
  // Each Dirac point is evaluated by exactly one processor (DiracKernelBase::addPoint ignores
  // points in elements owned by other processors), so every point's flux is nonzero on at most
  // one processor and a simple element-wise sum gathers the whole line sink.  However, the point
  // count itself is only guaranteed to agree across processors for point_file/reporter-defined
  // geometries; a line_base geometry on a distributed mesh can, in principle, produce a
  // different point count per processor, so guard against that before summing.
  std::size_t num_pts = _fluxes.size();
  gatherMax(num_pts);
  if (num_pts != _fluxes.size())
    mooseError("PorousFlowPointFluxQuantity: the number of line-sink points differs between "
               "processors (this processor has ",
               _fluxes.size(),
               ", another has ",
               num_pts,
               ").  This can happen when a line sink is defined using line_base on a distributed "
               "mesh: try mesh_mode = replicated.");

  gatherSum(_fluxes);
}
