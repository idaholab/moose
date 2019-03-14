#include "FlowChannel.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "TwoPhaseFluidProperties.h"
#include "VaporMixtureFluidProperties.h"
#include "TwoPhaseNCGFluidProperties.h"
#include "InputParameterLogic.h"
#include "StabilizationSettings.h"
#include "HeatTransferBase.h"
#include "HeatTransferBase1Phase.h"
#include "HeatTransferBase2Phase.h"
#include "ClosuresBase.h"

#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"

#include "THMMesh.h"
#include "THMApp.h"

registerMooseObject("THMApp", FlowChannel);

const std::map<std::string, FlowChannel::EConvHeatTransGeom>
    FlowChannel::_heat_transfer_geom_to_enum{{"PIPE", PIPE}, {"ROD_BUNDLE", ROD_BUNDLE}};

MooseEnum
FlowChannel::getConvHeatTransGeometry(const std::string & name)
{
  return THM::getMooseEnum<EConvHeatTransGeom>(name, _heat_transfer_geom_to_enum);
}

template <>
FlowChannel::EConvHeatTransGeom
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<FlowChannel::EConvHeatTransGeom>(s, FlowChannel::_heat_transfer_geom_to_enum);
}

template <>
InputParameters
validParams<FlowChannel>()
{
  InputParameters params = validParams<GeometricalFlowComponent>();
  params.addRequiredParam<FunctionName>("A", "Area of the pipe, can be a constant or a function");
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
                             FlowChannel::getConvHeatTransGeometry("PIPE"),
                             "Convective heat transfer geometry");
  params.addParam<Real>("PoD", 1, "pitch to diameter ratio for parallel bundle heat transfer");

  params.addParam<FunctionName>("initial_p", "Initial pressure in the pipe");
  params.addParam<FunctionName>("initial_p_liquid",
                                "Initial pressure in the pipe for the liquid phase");
  params.addParam<FunctionName>("initial_p_vapor",
                                "Initial pressure in the pipe for the vapor phase");
  params.addParam<FunctionName>("initial_vel", "Initial velocity in the pipe");
  params.addParam<FunctionName>("initial_vel_liquid",
                                "Initial velocity in the pipe for the liquid phase");
  params.addParam<FunctionName>("initial_vel_vapor",
                                "Initial velocity in the pipe for the vapor phase");
  params.addParam<FunctionName>("initial_T", "Initial temperature in the pipe");
  params.addParam<FunctionName>("initial_T_liquid",
                                "Initial temperature in the pipe for the liquid phase");
  params.addParam<FunctionName>("initial_T_vapor",
                                "Initial temperature in the pipe for the vapor phase");
  params.addParam<FunctionName>("initial_alpha_vapor", "Initial vapor volume fraction in the pipe");
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
  params.addParam<bool>("phase_interaction", true, "Phase interaction on");
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

  params.addPrivateParam<std::string>("component_type", "pipe");
  return params;
}

FlowChannel::FlowChannel(const InputParameters & params)
  : GeometricalFlowComponent(params),
    _flow_model(nullptr),
    _area_function(getParam<FunctionName>("A")),
    _closures_name(getParam<std::string>("closures")),
    _const_A(false),
    _pipe_pars_transferred(getParam<bool>("pipe_pars_transferred")),
    _has_Dh(isParamValid("D_h")),
    _Dh_function(_has_Dh ? getParam<FunctionName>("D_h") : ""),
    _roughness(getParam<Real>("roughness")),
    _HT_geometry(getEnumParam<EConvHeatTransGeom>("heat_transfer_geom")),
    _PoD(getParam<Real>("PoD")),
    _has_PoD(isParamValid("PoD")),
    _const_f_interface(isParamValid("f_interface")),
    _f_interface(_const_f_interface ? getParam<Real>("f_interface") : _zero),
    _is_horizontal(_gravity_angle_type == HORIZONTAL || _gravity_angle_type == MOSTLY_HORIZONTAL),
    _stabilization_uo_name(getParam<UserObjectName>("stabilization")),
    _temperature_mode(false),
    _n_heat_transfer_connections(0)
{
}

std::shared_ptr<const FlowModel>
FlowChannel::getFlowModel() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _flow_model;
}

std::shared_ptr<FlowModel>
FlowChannel::buildFlowModel()
{
  const std::string class_name = _app.getFlowModelClassName(_model_id);
  InputParameters pars = _factory.getValidParams(class_name);
  pars.set<Simulation *>("_sim") = &_sim;
  pars.set<FlowChannel *>("_pipe") = this;
  pars.set<UserObjectName>("fp") = _fp_name;
  pars.set<UserObjectName>("numerical_flux") = _numerical_flux_name;
  pars.set<AuxVariableName>("A_linear_name") = _A_linear_name;
  pars.set<MooseEnum>("rdg_slope_reconstruction") = _rdg_slope_reconstruction;
  if (_model_id == THM::FM_TWO_PHASE || _model_id == THM::FM_TWO_PHASE_NCG)
  {
    pars.set<bool>("phase_interaction") = getParam<bool>("phase_interaction");
    pars.set<bool>("pressure_relaxation") = getParam<bool>("pressure_relaxation");
    pars.set<bool>("velocity_relaxation") = getParam<bool>("velocity_relaxation");
    pars.set<bool>("interface_transfer") = getParam<bool>("interface_transfer");
    pars.set<bool>("wall_mass_transfer") = getParam<bool>("wall_mass_transfer");
    pars.set<UserObjectName>("rdg_int_var_uo_name") = _rdg_int_var_uo_name;
  }
  return _factory.create<FlowModel>(class_name, name(), pars, 0);
}

void
FlowChannel::init()
{
  GeometricalFlowComponent::init();

  _flow_model = buildFlowModel();
  _flow_model->init();

  _closures = buildClosures();

  _const_A = !_sim.hasFunction(_area_function);

  // apply logic for parameters with one- and two-phase variants
  bool isTwoPhase = _model_id == THM::FM_TWO_PHASE || _model_id == THM::FM_TWO_PHASE_NCG;
  getOneOrTwoPhaseParameters<FunctionName>(
      isTwoPhase, "initial_p", {"initial_p_liquid", "initial_p_vapor"}, *this);
  getOneOrTwoPhaseParameters<FunctionName>(
      isTwoPhase, "initial_T", {"initial_T_liquid", "initial_T_vapor"}, *this);
  getOneOrTwoPhaseParameters<FunctionName>(
      isTwoPhase, "initial_vel", {"initial_vel_liquid", "initial_vel_vapor"}, *this);

  // initialize the stabilization object
  if (!_stabilization_uo_name.empty())
    if (_sim.hasUserObject(_stabilization_uo_name))
    {
      StabilizationSettings & stabilization = const_cast<StabilizationSettings &>(
          _sim.getUserObject<StabilizationSettings>(_stabilization_uo_name));
      stabilization.initMooseObjects(*_flow_model);
    }
}

std::shared_ptr<ClosuresBase>
FlowChannel::buildClosures()
{
  auto thm_app = dynamic_cast<THMApp *>(&_app);
  const std::string class_name = thm_app->getClosuresClassName(_closures_name, _model_id);
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Simulation *>("_sim") = &_sim;
  return _factory.create<ClosuresBase>(class_name, genName(name(), class_name), params);
}

void
FlowChannel::initSecondary()
{
  GeometricalFlowComponent::initSecondary();

  // Determine heat transfer mode based on connected heat transfer components;
  // if at least one heat transfer component of temperature component is
  // connected, then it's temperature mode. Otherwise, it's heat flux mode, even
  // if no heat transfer components at all were provided - in that case, a zero
  // heat flux is added.
  for (unsigned int i = 0; i < _heat_transfer_names.size(); i++)
  {
    const HeatTransferBase & heat_transfer =
        getComponentByName<HeatTransferBase>(_heat_transfer_names[i]);
    if (heat_transfer.isTemperatureType())
      _temperature_mode = true;
  }
}

void
FlowChannel::check() const
{
  GeometricalFlowComponent::check();

  _closures->check(*this);

  // check that stabilization exists
  if (!_stabilization_uo_name.empty())
    if (!_sim.hasUserObject(_stabilization_uo_name))
      logWarning("Requested stabilization '", _stabilization_uo_name, "' does not exist. Typo?");

  // check types of heat transfer for all sources; must be all of same type
  if (_temperature_mode)
    for (unsigned int i = 0; i < _heat_transfer_names.size(); i++)
    {
      const HeatTransferBase & heat_transfer =
          getComponentByName<HeatTransferBase>(_heat_transfer_names[i]);
      if (!heat_transfer.isTemperatureType())
        logError("Heat sources for a flow channel must be all of temperature type or all of heat "
                 "flux type");
    }

  if (_model_id == THM::FM_SINGLE_PHASE)
  {
    bool ics_set =
        isParamValid("initial_p") && isParamValid("initial_T") && isParamValid("initial_vel");

    if (!ics_set && !_app.isRestarting())
    {
      // create a list of the missing IC parameters
      const std::vector<std::string> ic_params{"initial_p", "initial_T", "initial_vel"};
      std::ostringstream oss;
      for (const auto & ic_param : ic_params)
        if (!isParamValid(ic_param))
          oss << " " << ic_param;

      logError("The following initial condition parameters are missing:", oss.str());
    }
  }
  else if (_model_id == THM::FM_TWO_PHASE || _model_id == THM::FM_TWO_PHASE_NCG)
  {
    // check number of vapor volume fraction bound entries
    std::vector<Real> alpha_vapor_bounds = getParam<std::vector<Real>>("alpha_vapor_bounds");
    if (alpha_vapor_bounds.size() != 2)
      logError("The parameter 'alpha_vapor_bounds' should have exactly 2 entries (the first for "
               "the lower bound and the second for the upper bound), but the number of "
               "supplied entries was: ",
               alpha_vapor_bounds.size(),
               ".");

    // determine if any IC parameters are missing
    std::vector<std::string> ic_params{"initial_alpha_vapor",
                                       "initial_p_liquid",
                                       "initial_T_liquid",
                                       "initial_vel_liquid",
                                       "initial_p_vapor",
                                       "initial_T_vapor",
                                       "initial_vel_vapor"};
    if (_model_id == THM::FM_TWO_PHASE_NCG)
    {
      const TwoPhaseNCGFluidProperties & fp =
          _sim.getUserObject<TwoPhaseNCGFluidProperties>(_fp_name);
      unsigned int n_ncgs = fp.getNumberOfNCGs();

      ic_params.push_back("initial_x_ncgs");

      // check the number of provided ICs for NCG mass fractions
      if (isParamValid("initial_x_ncgs"))
      {
        const std::vector<FunctionName> initial_x_ncgs =
            getParam<std::vector<FunctionName>>("initial_x_ncgs");
        if (initial_x_ncgs.size() != n_ncgs)
          logError(
              "The provided number of ICs in the IC parameter 'initial_x_ncgs' (",
              initial_x_ncgs.size(),
              ") does not equal the number of non-condensable gases specified by the flow model (",
              n_ncgs,
              ").");
      }
    }
    else if (isParamValid("initial_x_ncgs"))
      logError("The parameter 'initial_x_ncgs' can only be supplied if there are non-condensable "
               "gases.");

    std::ostringstream missing_ics_oss;
    bool ics_set = true;
    for (const auto & ic_param : ic_params)
      if (!isParamValid(ic_param))
      {
        missing_ics_oss << " " << ic_param;
        ics_set = false;
      }

    if (!ics_set && !_app.isRestarting())
      logError("The following initial condition parameters are missing:", missing_ics_oss.str());
  }

  if (FlowModel::getSpatialDiscretizationType() == FlowModel::rDG)
  {
    if (_pars.isParamSetByUser("stabilization"))
      logError("The parameter 'stabilization' is not valid for rDG spatial discretization");

    if (_model_id != THM::FM_SINGLE_PHASE && !getParam<bool>("phase_interaction"))
      logError("The parameter 'phase_interaction' must be equal to 'true' for rDG spatial "
               "discretization.");

    if (_model_id == THM::FM_TWO_PHASE_NCG)
      logSpatialDiscretizationNotImplementedError(FlowModel::getSpatialDiscretizationType());
  }
}

void
FlowChannel::buildMeshNodes()
{
  MeshBase & the_mesh = _mesh.getMesh();

  Point p(0, 0, 0);
  for (unsigned int i = 0; i < _node_locations.size(); i++)
  {
    p(0) = _node_locations[i];
    const Node * nd = the_mesh.add_point(p);
    _node_ids.push_back(nd->id());
  }
}

void
FlowChannel::buildMesh()
{
  buildMeshNodes();

  MeshBase & the_mesh = _mesh.getMesh();
  BoundaryInfo & boundary_info = the_mesh.get_boundary_info();

  // create nodeset for all nodes for this component
  _nodeset_id = getNextBoundaryId();
  _nodeset_name = name();
  boundary_info.nodeset_name(_nodeset_id) = _nodeset_name;

  // Check that the number of nodes is consistent with the number of nodes in case component
  // developers screw up (typically in buildMeshNodes() call)
  if (usingSecondOrderMesh())
  {
    if (_node_ids.size() != (2 * _n_elem + 1))
      mooseError(name(),
                 ": Inconsistent number of nodes and elements. You have ",
                 _n_elem,
                 " elements and ",
                 _node_ids.size(),
                 " nodes.");
  }
  else
  {
    if (_node_ids.size() != _n_elem + 1)
      mooseError(name(),
                 ": Inconsistent number of nodes and elements. You have ",
                 _n_elem,
                 " elements and ",
                 _node_ids.size(),
                 " nodes.");
  }

  for (auto & node_id : _node_ids)
  {
    const Node * nd = the_mesh.node_ptr(node_id);
    boundary_info.add_node(nd, _nodeset_id);
  }

  // elems
  _subdomain_id = getNextSubdomainId();
  setSubdomainInfo(_subdomain_id, name());
  unsigned int bc_id_in = getNextBoundaryId();  // boundary id for inlet
  unsigned int bc_id_out = getNextBoundaryId(); // boundary id for outlet
  for (unsigned int i = 0; i < _n_elem; i++)
  {
    Elem * elem = nullptr;
    if (usingSecondOrderMesh())
    {
      elem = the_mesh.add_elem(new Edge3);
      elem->set_node(0) = the_mesh.node_ptr(_node_ids[2 * i]);
      elem->set_node(1) = the_mesh.node_ptr(_node_ids[2 * i + 2]);
      elem->set_node(2) = the_mesh.node_ptr(_node_ids[2 * i + 1]);
    }
    else
    {
      elem = the_mesh.add_elem(new Edge2);
      elem->set_node(0) = the_mesh.node_ptr(_node_ids[i]);
      elem->set_node(1) = the_mesh.node_ptr(_node_ids[i + 1]);
    }
    elem->subdomain_id() = _subdomain_id;
    _elem_ids.push_back(elem->id());

    // BCs
    if (i == 0)
    {
      Point pt = _position;
      _connections[FlowConnection::IN].push_back(Connection(pt, elem->node_ptr(0), bc_id_in, -1));
      boundary_info.add_side(elem, 0, bc_id_in);
      _mesh.setBoundaryName(bc_id_in, genName(name(), "in"));
    }
    if (i == (_n_elem - 1))
    {
      Point pt = _position + _length * _dir;
      _connections[FlowConnection::OUT].push_back(Connection(pt, elem->node_ptr(1), bc_id_out, 1));
      boundary_info.add_side(elem, 1, bc_id_out);
      _mesh.setBoundaryName(bc_id_out, genName(name(), "out"));
    }
  }
}

void
FlowChannel::addVariables()
{
  // This should be called after initSecondary() because it relies on the names
  // generated in initSecondary() of heat transfer components
  getHeatTransferVariableNames();

  _flow_model->addVariables();

  // wall heat flux
  if (!_temperature_mode && _n_heat_transfer_connections != 1)
    _sim.addVariable(false, FlowModel::HEAT_FLUX_WALL, FlowModel::feType(), _subdomain_id);

  // total heat flux perimeter
  if (_n_heat_transfer_connections > 1)
  {
    const std::string class_name = "SumIC";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<VariableName>("variable") = FlowModel::HEAT_FLUX_PERIMETER;
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<VariableName>>("values") = _P_hf_names;
    _sim.addInitialCondition(class_name, genName(name(), "P_hf_ic"), params);
  }

  _flow_model->addInitialConditions();

  if (!_stabilization_uo_name.empty())
  {
    const StabilizationSettings & stabilization =
        _sim.getUserObject<StabilizationSettings>(_stabilization_uo_name);
    stabilization.addVariables(*_flow_model, _subdomain_id);
  }
}

void
FlowChannel::setupDh()
{
  const std::string nm = genName(name(), "D_h_material");

  if (_has_Dh)
  {
    const std::string class_name = "GenericFunctionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<std::string>>("prop_names") = {FlowModel::HYDRAULIC_DIAMETER};
    params.set<std::vector<FunctionName>>("prop_values") = {_Dh_function};
    _sim.addMaterial(class_name, nm, params);

    makeFunctionControllableIfConstant(_Dh_function, "D_h");
  }
  else
  {
    const std::string class_name = "HydraulicDiameterCircularMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    _sim.addMaterial(class_name, nm, params);
  }
}

void
FlowChannel::addFormLossObjects()
{
  if (isParamValid("K_prime"))
  {
    if (_model_id == THM::FM_SINGLE_PHASE)
    {
      const std::string class_name = "OneDMomentumFormLoss";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
      params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
      params.set<std::vector<VariableName>>("arhoEA") = {FlowModelSinglePhase::RHOEA};
      params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
      params.set<MaterialPropertyName>("alpha") = FlowModel::UNITY;
      params.set<MaterialPropertyName>("rho") = FlowModelSinglePhase::DENSITY;
      params.set<MaterialPropertyName>("vel") = FlowModelSinglePhase::VELOCITY;
      params.set<MaterialPropertyName>("2phase_multiplier") = FlowModel::UNITY;
      params.set<FunctionName>("K_prime") = getParam<FunctionName>("K_prime");
      _sim.addKernel(class_name, Component::genName(name(), class_name), params);
    }
    else if (_model_id == THM::FM_TWO_PHASE || _model_id == THM::FM_TWO_PHASE_NCG)
    {
      {
        const std::string class_name = "OneDMomentumFormLoss";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID;
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<std::vector<VariableName>>("arhoA") = {FlowModelTwoPhase::ALPHA_RHO_A_LIQUID};
        params.set<std::vector<VariableName>>("arhouA") = {FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID};
        params.set<std::vector<VariableName>>("arhoEA") = {FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID};
        params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
        params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
        params.set<MaterialPropertyName>("rho") = FlowModelTwoPhase::DENSITY_LIQUID;
        params.set<MaterialPropertyName>("vel") = FlowModelTwoPhase::VELOCITY_LIQUID;
        if (isParamValid("K_2phase_mult_liquid"))
          params.set<MaterialPropertyName>("2phase_multiplier") =
              getParam<MaterialPropertyName>("K_2phase_mult_liquid");
        else
          params.set<MaterialPropertyName>("2phase_multiplier") = FlowModel::UNITY;
        params.set<FunctionName>("K_prime") = getParam<FunctionName>("K_prime");
        _sim.addKernel(class_name, Component::genName(name(), class_name, "liquid"), params);
      }
      {
        const std::string class_name = "OneDMomentumFormLoss";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<NonlinearVariableName>("variable") = FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR;
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<std::vector<VariableName>>("arhoA") = {FlowModelTwoPhase::ALPHA_RHO_A_VAPOR};
        params.set<std::vector<VariableName>>("arhouA") = {FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR};
        params.set<std::vector<VariableName>>("arhoEA") = {FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};
        params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
        params.set<MaterialPropertyName>("alpha") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
        params.set<MaterialPropertyName>("rho") = FlowModelTwoPhase::DENSITY_VAPOR;
        params.set<MaterialPropertyName>("vel") = FlowModelTwoPhase::VELOCITY_VAPOR;
        if (isParamValid("K_2phase_mult_vapor"))
          params.set<MaterialPropertyName>("2phase_multiplier") =
              getParam<MaterialPropertyName>("K_2phase_mult_vapor");
        else
          params.set<MaterialPropertyName>("2phase_multiplier") = FlowModel::UNITY;
        params.set<FunctionName>("K_prime") = getParam<FunctionName>("K_prime");
        _sim.addKernel(class_name, Component::genName(name(), class_name, "vapor"), params);
      }
    }
  }
}

void
FlowChannel::addCommonObjects()
{
  // Wall friction, Dh, and Ph are common objects
  std::vector<VariableName> cv_rho(1, FlowModelSinglePhase::DENSITY);
  std::vector<VariableName> cv_rhou(1, FlowModelSinglePhase::MOMENTUM_DENSITY);
  std::vector<VariableName> cv_rhoE(1, FlowModelSinglePhase::TOTAL_ENERGY_DENSITY);

  ExecFlagEnum ts_execute_on(MooseUtils::getDefaultExecFlagEnum());
  ts_execute_on = {EXEC_TIMESTEP_BEGIN, EXEC_INITIAL};

  {
    std::string class_name = "DirectionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    _sim.addMaterial(class_name, genName(name(), "dir_mat"), params);
  }

  if (!_pipe_pars_transferred)
  {
    // Area
    if (FlowModel::getSpatialDiscretizationType() == FlowModel::rDG)
    {
      {
        std::string class_name = "FunctionAux";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<AuxVariableName>("variable") = _A_linear_name;
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<FunctionName>("function") = _area_function;
        params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
        const std::string aux_kernel_name = genName(name(), "area_linear_aux");
        _sim.addAuxKernel(class_name, aux_kernel_name, params);
        makeFunctionControllableIfConstant(_area_function, "Area");
      }
      {
        const std::string class_name = "CopyValueAux";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<AuxVariableName>("variable") = FlowModel::AREA;
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<std::vector<VariableName>>("source") = {_A_linear_name};
        params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
        const std::string aux_kernel_name = genName(name(), "area_aux");
        _sim.addAuxKernel(class_name, aux_kernel_name, params);
        makeFunctionControllableIfConstant(_area_function, "Area");
      }
    }
    else
    {
      std::string class_name = "FunctionAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = FlowModel::AREA;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<FunctionName>("function") = _area_function;
      params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
      const std::string aux_kernel_name = genName(name(), "area_linear_aux");
      _sim.addAuxKernel(class_name, aux_kernel_name, params);
      makeFunctionControllableIfConstant(_area_function, "Area");
    }

    setupDh();
  }

  ExecFlagEnum execute_on_initial_linear(MooseUtils::getDefaultExecFlagEnum());
  execute_on_initial_linear = {EXEC_INITIAL, EXEC_LINEAR};

  // total heat flux perimeter aux kernel
  if (_n_heat_transfer_connections > 1)
  {
    const std::string class_name = "SumAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = FlowModel::HEAT_FLUX_PERIMETER;
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<VariableName>>("values") = _P_hf_names;
    params.set<ExecFlagEnum>("execute_on") = execute_on_initial_linear;
    _sim.addAuxKernel(class_name, genName(name(), "P_hf_auxkernel"), params);
  }

  // weighted average wall heat flux aux kernel
  if (!_temperature_mode)
  {
    if (_n_heat_transfer_connections > 1)
    {
      const std::string class_name = "WeightedAverageMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<MaterialPropertyName>("prop_name") = FlowModel::HEAT_FLUX_WALL;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<std::vector<MaterialPropertyName>>("values") = _q_wall_names;
      params.set<std::vector<VariableName>>("weights") = _P_hf_names;
      _sim.addMaterial(class_name, genName(name(), class_name, FlowModel::HEAT_FLUX_WALL), params);
    }
    else if (_n_heat_transfer_connections == 0)
    {
      const std::string class_name = "ConstantMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::string>("property_name") = FlowModel::HEAT_FLUX_WALL;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<Real>("value") = 0;
      _sim.addMaterial(class_name, genName(name(), class_name, FlowModel::HEAT_FLUX_WALL), params);
    }
  }
}

void
FlowChannel::addMooseObjects()
{
  addCommonObjects();

  _flow_model->addMooseObjects();
  _closures->addMooseObjects(*this);

  addFormLossObjects();

  if (!_stabilization_uo_name.empty())
  {
    InputParameters pars = emptyInputParameters();
    pars.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    pars.set<Component *>("component") = _parent == nullptr ? this : _parent;
    pars.set<UserObjectName>("fp") = getParam<UserObjectName>("fp");
    pars.set<RealVectorValue>("gravity_vector") = _gravity_vector;

    const StabilizationSettings & stabilization =
        _sim.getUserObject<StabilizationSettings>(_stabilization_uo_name);
    stabilization.addMooseObjects(*_flow_model, pars);
  }
}

void
FlowChannel::addHeatTransferName(const std::string & name) const
{
  _heat_transfer_names.push_back(name);
  _n_heat_transfer_connections++;
}

void
FlowChannel::getHeatTransferVariableNames()
{
  for (unsigned int i = 0; i < _n_heat_transfer_connections; i++)
  {
    const HeatTransferBase & heat_transfer =
        getComponentByName<HeatTransferBase>(_heat_transfer_names[i]);

    const HeatTransferBase1Phase * heat_transfer_1phase = dynamic_cast<const HeatTransferBase1Phase *>(&heat_transfer);
    const HeatTransferBase2Phase * heat_transfer_2phase = dynamic_cast<const HeatTransferBase2Phase *>(&heat_transfer);
    if (heat_transfer_1phase != nullptr)
      _Hw_1phase_names.push_back(heat_transfer_1phase->getWallHeatTransferCoefficient1PhaseName());
    else if (heat_transfer_2phase != nullptr)
    {
      _Hw_liquid_names.push_back(heat_transfer_2phase->getWallHeatTransferCoefficientLiquidName());
      _Hw_vapor_names.push_back(heat_transfer_2phase->getWallHeatTransferCoefficientVaporName());
    }
    _P_hf_names.push_back(heat_transfer.getHeatedPerimeterName());
    _T_wall_names.push_back(heat_transfer.getWallTemperatureName());
    _q_wall_names.push_back(heat_transfer.getWallHeatFluxName());
  }
}

std::string
FlowChannel::getHeatTransferNamesSuffix(const std::string & ht_name) const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  // if there is more than one connected heat transfer component, then number them
  if (_n_heat_transfer_connections > 1)
  {
    // determine index of heat transfer name based on when it was added
    auto it = std::find(_heat_transfer_names.begin(), _heat_transfer_names.end(), ht_name);
    if (it != _heat_transfer_names.end())
    {
      const unsigned int index = std::distance(_heat_transfer_names.begin(), it);
      const std::string suffix = ":" + std::to_string(index + 1);
      return suffix;
    }
    else
      mooseError("Heat transfer component '", ht_name, "' was not added to pipe '", name(), "'");
  }
  // else, don't add a suffix; there is no need
  else
    return "";
}

const std::vector<unsigned int> &
FlowChannel::getNodeIDs() const
{
  checkSetupStatus(MESH_PREPARED);

  return _node_ids;
}

const std::vector<unsigned int> &
FlowChannel::getElementIDs() const
{
  checkSetupStatus(MESH_PREPARED);

  return _elem_ids;
}

unsigned int
FlowChannel::getNodesetID() const
{
  checkSetupStatus(MESH_PREPARED);

  return _nodeset_id;
}

const BoundaryName &
FlowChannel::getNodesetName() const
{
  checkSetupStatus(MESH_PREPARED);

  return _nodeset_name;
}

unsigned int
FlowChannel::getSubdomainID() const
{
  checkSetupStatus(MESH_PREPARED);

  return _subdomain_id;
}
