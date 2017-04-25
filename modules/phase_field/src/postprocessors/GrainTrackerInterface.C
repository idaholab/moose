/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GrainTrackerInterface.h"
#include "FeatureFloodCount.h"

template <>
InputParameters
validParams<GrainTrackerInterface>()
{
  InputParameters params = validParams<FeatureFloodCount>();
  params.addParam<int>("tracking_step", 0, "The timestep for when we should start tracking grains");
  params.addParam<unsigned int>(
      "halo_level", 2, "The thickness of the halo surrounding each feature.");
  params.addParam<bool>(
      "remap_grains", true, "Indicates whether remapping should be done or not (default: true)");
  params.addParam<unsigned short>(
      "reserve_op",
      0,
      "Indicates the number of reserved ops (variables that cannot be remapped to)");
  params.addParam<Real>("reserve_op_threshold",
                        0.95,
                        "Threshold for locating a new feature on the reserved op variable(s)");
  params.addParam<UserObjectName>("ebsd_reader", "Optional: EBSD Reader for initial condition");
  params.addParam<unsigned int>("phase", "EBSD phase number from which to retrieve information");
  params.addParam<bool>("error_on_grain_creation",
                        false,
                        "Terminate with an error if a grain is created "
                        "(does not include initial callback to start simulation)");

  params.addRequiredCoupledVarWithAutoBuild(
      "variable", "var_name_base", "op_num", "Array of coupled variables");

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

  MooseUtils::setExecuteOnFlags(params, {EXEC_INITIAL, EXEC_TIMESTEP_END});
  return params;
}

std::vector<unsigned int>
GrainTrackerInterface::getNewGrainIDs() const
{
  return std::vector<unsigned int>();
}
