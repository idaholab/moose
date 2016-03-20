/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "GrainTrackerInterface.h"
#include "FeatureFloodCount.h"

template<>
InputParameters validParams<GrainTrackerInterface>()
{
  InputParameters params = validParams<FeatureFloodCount>();
  params.addParam<int>("tracking_step", 0, "The timestep for when we should start tracking grains");
  params.addParam<Real>("convex_hull_buffer", 1.0, "The buffer around the convex hull used to determine"
                                                   "when features intersect");
  params.addParam<unsigned int>("halo_level", 1, "The thickness of the halo surrounding each feature.");

  params.addParam<bool>("remap_grains", true, "Indicates whether remapping should be done or not (default: true)");
  params.addParam<bool>("compute_op_maps", false, "Indicates whether the data structures that"
                                                  "hold the active order parameter information"
                                                  "should be populated or not");
  params.addParam<bool>("center_of_mass_tracking", false, "Indicates whether the grain tracker uses bounding sphere centers"
                                                          "or center of mass calculations for tracking grains");
  params.addParam<UserObjectName>("ebsd_reader", "Optional: EBSD Reader for initial condition");

  params.addRequiredCoupledVarWithAutoBuild("variable", "var_name_base", "op_num", "Array of coupled variables");

  params.suppressParameter<std::vector<VariableName> >("variable");
  return params;
}
