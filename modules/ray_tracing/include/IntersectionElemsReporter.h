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
#include "RayTracingOverlayMeshTest.h"

class IntersectionElemsReporter;

/**
 * Reports the summary table of solution invalid warnings
 */
class IntersectionElemsReporter : public GeneralReporter
{
public:
  static InputParameters validParams();

  IntersectionElemsReporter(const InputParameters & parameters);

  void initialize() override {}
  void finalize() override {}
  void execute() override {}
};

// Store solution invalid warnings to a json file
void to_json(nlohmann::json & json, const RayTracingOverlayMeshTest * const & overlay_mesh_test);
