#include "FlowChannel.h"
#include "FlowChannelBase.h"
#include "THMApp.h"

registerMooseObject("THMApp", FlowChannel);

template <>
InputParameters
validParams<FlowChannel>()
{
  InputParameters params = validParams<FlowChannelBase>();
  params.addRequiredParam<FunctionName>(
      "A", "Area of the flow channel, can be a constant or a function");
  params.addParam<Real>("roughness", 0.0, "roughness, [m]");
  params.addParam<FunctionName>("f", "Wall friction");
  params.addParam<MaterialPropertyName>("f_2phase_mult_liquid",
                                        "2-phase multiplier property for friction for liquid");
  params.addParam<MaterialPropertyName>("f_2phase_mult_vapor",
                                        "2-phase multiplier property for friction for vapor");

  params.addParam<FunctionName>("K_prime", "Form loss coefficient per unit length function");
  params.addParam<MaterialPropertyName>("K_2phase_mult_liquid",
                                        "2-phase multiplier property for form loss for liquid");
  params.addParam<MaterialPropertyName>("K_2phase_mult_vapor",
                                        "2-phase multiplier property for form loss for vapor");

  params.addParam<MooseEnum>("heat_transfer_geom",
                             FlowChannelBase::getConvHeatTransGeometry("PIPE"),
                             "Convective heat transfer geometry");
  params.addParam<Real>("PoD", 1, "pitch to diameter ratio for parallel bundle heat transfer");

  params.addParam<FunctionName>("initial_p", "Initial pressure in the flow channel");
  params.addParam<FunctionName>("initial_p_liquid",
                                "Initial pressure in the flow channel for the liquid phase");
  params.addParam<FunctionName>("initial_p_vapor",
                                "Initial pressure in the flow channel for the vapor phase");
  params.addParam<FunctionName>("initial_vel", "Initial velocity in the flow channel");
  params.addParam<FunctionName>("initial_vel_liquid",
                                "Initial velocity in the flow channel for the liquid phase");
  params.addParam<FunctionName>("initial_vel_vapor",
                                "Initial velocity in the flow channel for the vapor phase");
  params.addParam<FunctionName>("initial_T", "Initial temperature in the flow channel");
  params.addParam<FunctionName>("initial_T_liquid",
                                "Initial temperature in the flow channel for the liquid phase");
  params.addParam<FunctionName>("initial_T_vapor",
                                "Initial temperature in the flow channel for the vapor phase");
  params.addParam<FunctionName>("initial_alpha_vapor",
                                "Initial vapor volume fraction in the flow channel");
  params.addParam<std::vector<FunctionName>>("initial_x_ncgs",
                                             "Initial non-condensable gas mass fractions, if any");

  params.addParam<bool>(
      "pipe_pars_transferred",
      false,
      "Set to true if Dh, P_hf and A are going to be transferred in from an external source");
  params.addParam<FunctionName>("D_h", "Hydraulic diameter");

  params.addParam<UserObjectName>(
      "stabilization", "", "The name of the local stabilization scheme to use");
  params.addParam<bool>("shock_capturing", false, "Use shock capturing or not (locally)");
  params.addParam<Real>("f_interface", "interface friction");
  params.addParam<bool>("lump_mass_matrix", false, "Lump the mass matrix");

  params.addRequiredParam<std::string>("closures", "Closures type");

  params.addParam<std::string>("chf_table", "The lookup table used for critical heat flux");

  // bounds
  std::vector<Real> alpha_vapor_bounds(2, 0);
  alpha_vapor_bounds[0] = 0.0001;
  alpha_vapor_bounds[1] = 0.9999;
  params.addParam<std::vector<Real>>(
      "alpha_vapor_bounds", alpha_vapor_bounds, "Bounds imposed on the vapor volume fraction");
  params.addParam<Real>("volume_fraction_remapper_exponential_region_width",
                        1e-6,
                        "Width of the exponential regions in the volume fraction remapper");
  // 7-equation 2-phase flow global parameter
  params.addParam<bool>("pressure_relaxation", true, "Pressure relaxation on");
  params.addParam<bool>("velocity_relaxation", true, "True for using velocity relaxation");
  params.addParam<bool>("interface_transfer", true, "interface heat/mass transfer");
  params.addParam<bool>("wall_mass_transfer", true, "wall mass transfer on");

  params.addParam<Real>("specific_interfacial_area_max_value",
                        1700.0,
                        "the max value of the specific interfacial area");
  params.addParam<Real>("specific_interfacial_area_min_value",
                        0.03,
                        "The minimal value of the "
                        "specific interfacial area "
                        "(i.e. the value used in the "
                        "cut off part)");
  params.addParam<bool>(
      "explicit_acoustic_impedance", false, "if an explicit acoustic impedance should be used");
  params.addParam<bool>(
      "explicit_alpha_gradient", false, "if an explicit alpha gradient should be used");
  params.addParam<Real>("heat_exchange_coef_liquid",
                        "a user-given heat exchange coefficient of liquid");
  params.addParam<Real>("heat_exchange_coef_vapor",
                        "a user-given heat exchange coefficient of vapor");
  params.addParam<Real>("pressure_relaxation_rate",
                        "a user-given value for pressure relaxation rate");
  params.addParam<Real>("velocity_relaxation_rate",
                        "a user-given value for velocity relaxation rate");
  return params;
}

FlowChannel::FlowChannel(const InputParameters & params) : FlowChannelBase(params) {}

std::shared_ptr<FlowModel>
FlowChannel::buildFlowModel()
{
  const std::string class_name = _app.getFlowModelClassName(_model_id);
  InputParameters pars = _factory.getValidParams(class_name);
  pars.set<Simulation *>("_sim") = &_sim;
  pars.set<FlowChannelBase *>("_flow_channel") = this;
  pars.set<UserObjectName>("fp") = _fp_name;
  pars.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
  pars.set<MooseEnum>("rdg_slope_reconstruction") = _rdg_slope_reconstruction;
  if (_model_id == THM::FM_TWO_PHASE || _model_id == THM::FM_TWO_PHASE_NCG)
  {
    pars.set<bool>("pressure_relaxation") = getParam<bool>("pressure_relaxation");
    pars.set<bool>("velocity_relaxation") = getParam<bool>("velocity_relaxation");
    pars.set<bool>("interface_transfer") = getParam<bool>("interface_transfer");
    pars.set<bool>("wall_mass_transfer") = getParam<bool>("wall_mass_transfer");
    pars.set<UserObjectName>("rdg_int_var_uo_name") = _rdg_int_var_uo_name;
    pars.set<FunctionName>("initial_alpha_vapor") = getParam<FunctionName>("initial_alpha_vapor");
    pars.set<std::vector<Real>>("alpha_vapor_bounds") =
        _sim.getParamTempl<std::vector<Real>>("alpha_vapor_bounds");
    pars.set<Real>("volume_fraction_remapper_exponential_region_width") =
        _sim.getParamTempl<Real>("volume_fraction_remapper_exponential_region_width");
  }
  return _factory.create<FlowModel>(class_name, name(), pars, 0);
}

void
FlowChannel::check() const
{
  if (_model_id == THM::FM_SINGLE_PHASE)
    logError("FlowChannel component is deprecated, use 'type = FlowChannel1Phase' instead.");
  else if (_model_id == THM::FM_TWO_PHASE || _model_id == THM::FM_TWO_PHASE_NCG)
    logError("FlowChannel component is deprecated, use 'type = FlowChannel2Phase' instead.");
}
