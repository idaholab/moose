//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralRayBC.h"

// Forward declarations
class ViewFactorRayStudy;

/**
 * RayBC used in the computation of view factors using the angular quadrature
 * ray tracing method.
 */
class ViewFactorRayBC : public GeneralRayBC
{
public:
  ViewFactorRayBC(const InputParameters & params);

  static InputParameters validParams();

  void onBoundary(const unsigned int num_applying) override;

protected:
  /// The ViewFactorRayStudy
  ViewFactorRayStudy & _vf_study;

  /// Index in the Ray aux data for the starting boundary ID
  const RayDataIndex _ray_index_start_bnd_id;
  /// Index in the Ray aux data for the starting total weight (dot * qp weight)
  const RayDataIndex _ray_index_start_total_weight;
};
