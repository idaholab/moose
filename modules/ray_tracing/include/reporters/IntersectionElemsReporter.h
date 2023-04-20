//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "RayTracingOverlayMeshMapping.h"

class IntersectionElemsReporter;
class RayTracingOverlayMeshMapping;
/**
 * Reports the summary of the two-way mapping between main and overlay mesh
 */
class IntersectionElemsReporter : public GeneralReporter
{
public:
  static InputParameters validParams();

  IntersectionElemsReporter(const InputParameters & parameters);

  void initialize() override {}
  void finalize() override {}
  void execute() override final;

private:
  const bool _serialize;
  // The reference to RayTracingOverlayMeshMapping
  const RayTracingOverlayMeshMapping & _overlay_mesh_mapping;
  /**
   *  Store the intersection elem IDs in two-way maps
   */
  // main -> overlay map use main elem at the boundary as key
  std::map<dof_id_type, std::set<dof_id_type>> & _to_overlay;
  // overlay -> main map use overlay elem as key
  std::map<dof_id_type, std::set<dof_id_type>> & _from_overlay;
};
