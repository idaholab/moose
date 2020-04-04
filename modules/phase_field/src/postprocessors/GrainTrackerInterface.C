//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainTrackerInterface.h"
#include "FeatureFloodCount.h"

InputParameters
GrainTrackerInterface::validParams()
{
  InputParameters params = FeatureFloodCount::validParams();
  params.addParam<int>("tracking_step", 0, "The timestep for when we should start tracking grains");
  params.addParam<unsigned short>(
      "halo_level", 2, "The thickness of the halo surrounding each feature.");
  params.addParam<bool>(
      "remap_grains", true, "Indicates whether remapping should be done or not (default: true)");
  params.addParam<bool>("tolerate_failure",
                        false,
                        "Allow the grain tracker to continue when it fails to find suitable grains "
                        "for remapping. This will allow the simulation to continue but it will "
                        "also allow non-physical grain coalescence. DO NOT USE for production "
                        "results!");

  params.addParam<unsigned short>(
      "reserve_op",
      0,
      "Indicates the number of reserved ops (variables that cannot be remapped to)");
  params.addParam<Real>("reserve_op_threshold",
                        0.95,
                        "Threshold for locating a new feature on the reserved op variable(s)");
  params.addParam<UserObjectName>("polycrystal_ic_uo", "Optional: Polycrystal IC object");
  params.addParam<bool>("error_on_grain_creation",
                        false,
                        "Terminate with an error if a grain is created "
                        "(does not include initial callback to start simulation)");

  params.addParam<unsigned short>("max_remap_recursion_depth",
                                  6,
                                  "The recursion depth allowed when searching for remapping "
                                  "candidates. Note: Setting this value high may result in very "
                                  "computationally expensive searches with little benefit. "
                                  "(Recommended level: 6)");

  params.addRangeCheckedParam<short>(
      "verbosity_level",
      1,
      "verbosity_level>=0 & verbosity_level<=3",
      "Level 0: Silent during normal operation, "
      "Level 1: Informational messages but no detailed grain structure printouts, "
      "Level 2: Verbose output including data structure dumps, "
      "Level 3: Debugging mode.");

  params.addRequiredCoupledVarWithAutoBuild(
      "variable", "var_name_base", "op_num", "Array of coupled variables");

  params.addParamNamesToGroup("tolerate_failure max_remap_recursion_depth", "Advanced");

  // Set suitable default parameters for grain tracking
  params.set<Real>("threshold") = 0.1; // flood out to a fairly low value for grain remapping
  params.set<Real>("connecting_threshold") =
      0.09; // connecting threshold should be just slightly lower than threshold
  params.set<bool>("use_single_map") =
      false; // This is needed to allow for arbitrary feature overlap during remapping
  params.set<bool>("condense_map_info") =
      true; // It's better to have all information in one map for normal visualization
  params.set<bool>("enable_var_coloring") =
      true; // Generally we need to see the variable (OP) indices

  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  return params;
}

std::vector<unsigned int>
GrainTrackerInterface::getNewGrainIDs() const
{
  return std::vector<unsigned int>();
}
