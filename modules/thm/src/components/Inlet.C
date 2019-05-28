#include "Inlet.h"
#include "FlowModelTwoPhase.h"

registerMooseObject("THMApp", Inlet);

template <>
InputParameters
validParams<Inlet>()
{
  InputParameters params = validParams<FlowBoundary>();
  // T, m^dot
  params.addParam<Real>("m_dot", "Prescribed mass flow rate");
  params.addParam<Real>("T", "prescribed temperature (used only in 3eqn model)");
  // static p and T
  params.addParam<Real>("p", "prescribed pressure");
  // mass flow rate
  params.addParam<Real>("rho", "Prescribed density");
  params.addParam<Real>("vel", "Prescribed velocity");
  // H, rhou
  params.addParam<Real>("rhou", "Prescribed momentum");
  params.addParam<Real>("H", "Prescribed specific total enthalpy");
  // stagnation parameters
  params.addParam<Real>("p0", "Prescribed stagnation pressure");
  params.addParam<Real>("T0", "Prescribed stagnation temperature");

  // 2-phase
  // mass flow rate
  params.addParam<Real>("rho_liquid", "Prescribed density of liquid");
  params.addParam<Real>("vel_liquid", "Prescribed velocity of liquid");
  params.addParam<Real>("rho_vapor", "Prescribed density of vapor");
  params.addParam<Real>("vel_vapor", "Prescribed velocity of vapor");
  // H, rhou
  params.addParam<Real>("rhou_liquid", "Prescribed momentum for liquid");
  params.addParam<Real>("rhou_vapor", "Prescribed momentum for vapor");
  params.addParam<Real>("H_liquid", "Prescribed specific total enthalpy for liquid");
  params.addParam<Real>("H_vapor", "Prescribed specific total enthalpy for vapor");
  // m_dot, T
  params.addParam<Real>("m_dot_liquid", "Prescribed mass flow rate of liquid");
  params.addParam<Real>("T_liquid", "Prescribed temperature of liquid");
  params.addParam<Real>("m_dot_vapor", "Prescribed mass flow rate of vapor");
  params.addParam<Real>("T_vapor", "Prescribed temperature of vapor");

  // static p and T
  params.addParam<Real>("p_liquid", "");
  params.addParam<Real>("p_vapor", "");
  // stagnation parameters
  params.addParam<Real>("p0_liquid", "Prescribed stagnation pressure for liquid phase");
  params.addParam<Real>("T0_liquid", "Prescribed stagnation temperature for liquid phase");
  params.addParam<Real>("p0_vapor", "Prescribed stagnation pressure for vapor phase");
  params.addParam<Real>("T0_vapor", "Prescribed stagnation temperature for vapor phase");

  params.addParam<Real>("alpha_vapor", "Prescribed vapor volume fraction");

  params.addParam<bool>("reversible", false, "true for reversible, false (default) for pure inlet");

  return params;
}

Inlet::Inlet(const InputParameters & params) : FlowBoundary(params) {}

void
Inlet::check() const
{
  // Input type checking
  std::set<BoundaryType> bct;
  if (_flow_model_id == THM::FM_SINGLE_PHASE)
  {
    if (isParamValid("p") && isParamValid("T"))
      bct.insert(BC_TYPE_STATIC_PT);
    if (isParamValid("p0") && isParamValid("T0"))
      bct.insert(BC_TYPE_STAGNATION_PT);
    if (isParamValid("rho") && isParamValid("vel"))
      bct.insert(BC_TYPE_DENSITY_VELOCITY);
    if (isParamValid("rhou") && isParamValid("H"))
      bct.insert(BC_TYPE_H_RHOU);
    if (isParamValid("m_dot") && isParamValid("T"))
      bct.insert(BC_TYPE_MASS_FLOW_RATE);
  }
  else if (_flow_model_id == THM::FM_TWO_PHASE)
  {
    if (isParamValid("p_liquid") && isParamValid("T_liquid") && isParamValid("p_vapor") &&
        isParamValid("T_vapor") && isParamValid("alpha_vapor"))
      bct.insert(BC_TYPE_STATIC_PT);
    if (isParamValid("p0_liquid") && isParamValid("T0_liquid") && isParamValid("p0_vapor") &&
        isParamValid("T0_vapor") && isParamValid("alpha_vapor"))
      bct.insert(BC_TYPE_STAGNATION_PT);
    if (isParamValid("vel_liquid") && isParamValid("vel_vapor") && isParamValid("rho_liquid") &&
        isParamValid("rho_vapor") && isParamValid("alpha_vapor"))
      bct.insert(BC_TYPE_DENSITY_VELOCITY);
    if (isParamValid("rhou_liquid") && isParamValid("rhou_vapor") && isParamValid("H_liquid") &&
        isParamValid("H_vapor"))
      bct.insert(BC_TYPE_H_RHOU);
    if (isParamValid("m_dot_liquid") && isParamValid("T_liquid") && isParamValid("m_dot_vapor") &&
        isParamValid("T_vapor") && isParamValid("alpha_vapor"))
      bct.insert(BC_TYPE_MASS_FLOW_RATE);

    const FlowModelTwoPhase & fm = dynamic_cast<const FlowModelTwoPhase &>(*_flow_model);
    bool phase_interaction = fm.getPhaseInteraction();
    if (phase_interaction && !isParamValid("alpha_vapor"))
      logError("The 7-eqn model requires 'alpha_vapor' parameter to be specified.");
  }
  else
  {
    logError("The specified model type (", _flow_model_id, ") is not supported.");
  }

  // Input error checking
  if (bct.empty())
  {
    logError("Inlet component is deprecated and will be removed on Oct 1, 2019. Please update your "
             "input file.");
    logError("Boundary conditions are under-specified.");
  }
  else if (bct.size() > 1)
  {
    logError("Inlet component is deprecated and will be removed on Oct 1, 2019. Please update your "
             "input file.");
    logError("Boundary conditions are overspecified.");
  }
  else
  {
    BoundaryType bc_type = *bct.begin();

    if (bc_type == BC_TYPE_STATIC_PT)
      logError(
          "Static p and T BC has been deprecated (will be removed on Oct 1, 2019) and should not "
          "be used. Use 'type = InletDensityVelocity' or 'type = "
          "InletStagnationPressureTemperature' instead.");
    else if (bc_type == BC_TYPE_STAGNATION_PT)
      logError("Inlet component is obsolete (will be removed on Oct 1, 2019), use 'type = "
               "InletStagnationPressureTemperature' instead.");
    else if (bc_type == BC_TYPE_MASS_FLOW_RATE)
      logError("Inlet component is obsolete (will be removed on Oct 1, 2019), use 'type = "
               "InletMassFlowRateTemperature' instead.");
    else if (bc_type == BC_TYPE_DENSITY_VELOCITY)
      logError("Inlet component is obsolete (will be removed on Oct 1, 2019), use 'type = "
               "InletDensityVelocity' instead.");
    else if (bc_type == BC_TYPE_H_RHOU)
      logError("Inlet component is obsolete (will be removed on Oct 1, 2019), use 'type = "
               "InletStagnationEnthalpyMomentum' instead.");
    else
      logError("Boundary condition of type '",
               bc_type,
               "' has not been implemented for model_type ",
               _flow_model_id);
  }

  if (_spatial_discretization == FlowModel::rDG)
    logSpatialDiscretizationNotImplementedError(_spatial_discretization);
}
