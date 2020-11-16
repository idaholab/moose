#include "THMProblem.h"

registerMooseObject("THMApp", THMProblem);

InputParameters
THMProblem::validParams()
{
  InputParameters params = FEProblem::validParams();

  // default scaling factors
  std::vector<Real> sf_1phase(3, 1.0);
  params.addParam<std::vector<Real>>(
      "scaling_factor_1phase",
      sf_1phase,
      "Scaling factors for each single phase variable (rhoA, rhouA, rhoEA)");
  std::vector<Real> sf_2phase(7, 1.);
  params.addParam<std::vector<Real>>("scaling_factor_2phase",
                                     sf_2phase,
                                     "Scaling factors for each variable of 7eqn 2phase model "
                                     "(alpha_l, arhoA_l, arhouA_l, arhoEA_l, "
                                     "arhoA_v, arhouA_v, arhoEA_v");
  params.addParam<std::vector<Real>>(
      "scaling_factor_ncgs", "Scaling factor(s) for the non-condensable gas equations, if any");
  params.addParam<Real>(
      "scaling_factor_temperature", 1.0, "Scaling factor for solid temperature variable.");

  params.addParam<MooseEnum>("spatial_discretization",
                             FlowModel::getSpatialDiscretizationMooseEnum("rDG"),
                             "Spatial discretization");
  params.addParam<bool>("2nd_order_mesh", false, "Use 2nd order elements in the mesh");

  // bounds
  std::vector<Real> alpha_vapor_bounds(2, 0);
  alpha_vapor_bounds[0] = 0.0001;
  alpha_vapor_bounds[1] = 0.9999;
  params.addParam<std::vector<Real>>(
      "alpha_vapor_bounds", alpha_vapor_bounds, "Bounds imposed on the vapor volume fraction");
  params.addParam<Real>("volume_fraction_remapper_exponential_region_width",
                        1e-6,
                        "Width of the exponential regions in the volume fraction remapper");

  params.addParam<FileName>("initial_from_file",
                            "The name of an exodus file with initial conditions");
  params.addParam<std::string>(
      "initial_from_file_timestep",
      "LATEST",
      "Gives the timestep (or \"LATEST\") for which to read a solution from a file "
      "for a given variable. (Default: LATEST)");

  params.addClassDescription("Specialization of FEProblem to run with component subsystem");

  return params;
}

THMProblem::THMProblem(const InputParameters & parameters)
  : FEProblem(parameters), Simulation(*this, parameters)
{
}

void
THMProblem::advanceState()
{
  FEProblem::advanceState();
  Simulation::advanceState();
}

bool
THMProblem::hasPostprocessor(const std::string & name) const
{
  ReporterName r_name(name, "value");
  return _reporter_data.hasReporterValue(r_name);
}
