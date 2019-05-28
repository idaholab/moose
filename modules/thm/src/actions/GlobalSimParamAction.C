#include "GlobalSimParamAction.h"
#include "Simulation.h"
#include "FlowModel.h"
#include "HeatConductionModel.h"
#include "THMApp.h"
#include "Numerics.h"

registerMooseAction("THMApp", GlobalSimParamAction, "THM:global_sim_params");

template <>
InputParameters
validParams<GlobalSimParamAction>()
{
  InputParameters params = validParams<THMAction>();
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
                             FlowModel::getSpatialDiscretizationMooseEnum("CG"),
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

  return params;
}

GlobalSimParamAction::GlobalSimParamAction(InputParameters parameters) : THMAction(parameters)
{
  InputParameters & pars = _simulation.params();
  // merge parameters into simulation parameters
  pars += this->parameters();

  // Setting the _spatial_discretization and _flow_fe_type members is currently achieved
  // via friend statement. This is because we need to support CG.  When CG is removed
  // the need to friend should go away
  _simulation._spatial_discretization = THM::stringToEnum<FlowModel::ESpatialDiscretizationType>(
      pars.get<MooseEnum>("spatial_discretization"));

  bool second_order_mesh = pars.get<bool>("2nd_order_mesh");
  HeatConductionModel::_fe_type =
      second_order_mesh ? FEType(SECOND, LAGRANGE) : FEType(FIRST, LAGRANGE);
  if (_simulation.getSpatialDiscretization() == FlowModel::CG)
    _simulation._flow_fe_type = HeatConductionModel::_fe_type;
}

void
GlobalSimParamAction::act()
{
}
