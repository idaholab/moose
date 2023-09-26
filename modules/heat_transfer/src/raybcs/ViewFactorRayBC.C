//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ViewFactorRayBC.h"
#include "ViewFactorRayStudy.h"

registerMooseObject("HeatConductionApp", ViewFactorRayBC);

InputParameters
ViewFactorRayBC::validParams()
{
  InputParameters params = GeneralRayBC::validParams();
  params.addClassDescription("This ray boundary condition is applied on all sidesets bounding a "
                             "radiation cavity except symmetry sidesets. It kills rays that hit "
                             "the sideset and scores the ray for computation of view factors.");
  return params;
}

ViewFactorRayBC::ViewFactorRayBC(const InputParameters & params)
  : GeneralRayBC(params),
    _vf_study(getStudy<ViewFactorRayStudy>()),
    _ray_index_start_bnd_id(_vf_study.rayIndexStartBndID()),
    _ray_index_start_total_weight(_vf_study.rayIndexStartTotalWeight())
{
}

void
ViewFactorRayBC::onBoundary(const unsigned int num_applying)
{
  // The boundary ID this Ray started on
  const BoundaryID start_bnd_id = currentRay()->auxData(_ray_index_start_bnd_id);
  // Starting total weight
  const Real start_total_weight = currentRay()->auxData(_ray_index_start_total_weight);
  // Value to append (divide by num_applying if we hit an edge or node)
  const Real value = start_total_weight / (Real)num_applying;
  mooseAssert(!std::isnan(value), "Encountered NaN");

  // Accumulate into the view factor info
  _vf_study.addToViewFactorInfo(value, start_bnd_id, _current_bnd_id, _tid);

  // Either hit an obstacle here or hit its end and contributed: done with this Ray
  currentRay()->setShouldContinue(false);
}
