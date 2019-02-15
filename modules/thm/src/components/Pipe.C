#include "Pipe.h"

#include "TwoPhaseFluidProperties.h"
#include "VaporMixtureFluidProperties.h"
#include "TwoPhaseNCGFluidProperties.h"

#include "InputParameterLogic.h"

#include "StabilizationSettings.h"
#include "FlowModelSinglePhase.h"
#include "FlowModelTwoPhase.h"
#include "FlowModelTwoPhaseNCG.h"
#include "HeatTransferBase.h"
#include "ClosuresBase.h"

#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"

#include "THMMesh.h"
#include "THMApp.h"

registerMooseObject("THMApp", Pipe);

template <>
InputParameters
validParams<Pipe>()
{
  InputParameters params = validParams<PipeBase>();

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
                             PipeBase::getConvHeatTransGeometry("PIPE"),
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

  std::string closures;
  for (auto && c : THMApp::closuresOptions())
    closures += c + " ";
  MooseEnum closures_type(closures, THMApp::defaultClosuresOption());
  params.addParam<MooseEnum>("closures_type", closures_type, "Closures type");

  std::string chf_tables;
  for (auto && c : THMApp::criticalHeatFluxTableTypes())
    chf_tables += c + " ";
  MooseEnum chf_table_type(chf_tables, THMApp::defaultCriticalHeatFluxTableType());
  params.addParam<MooseEnum>(
      "chf_table", chf_table_type, "The lookup table used for critical heat flux");

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

  return params;
}

Pipe::Pipe(const InputParameters & params)
  : PipeBase(params),
    _closures_name(getParam<MooseEnum>("closures_type")),
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

void
Pipe::init()
{
  PipeBase::init();

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
  getOneOrTwoPhaseParameters<FunctionName>(isTwoPhase, "Hw", {"Hw_liquid", "Hw_vapor"}, *this);

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
Pipe::buildClosures()
{
  auto thm_app = dynamic_cast<THMApp *>(&_app);
  const std::string class_name = thm_app->getClosuresClassName(_closures_name, _model_id);
  InputParameters params = _factory.getValidParams(class_name);
  params.set<Simulation *>("_sim") = &_sim;
  return _factory.create<ClosuresBase>(class_name, genName(name(), class_name), params);
}

void
Pipe::initSecondary()
{
  PipeBase::initSecondary();

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
Pipe::check() const
{
  PipeBase::check();

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
Pipe::buildMeshNodes()
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
Pipe::buildMesh()
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
Pipe::addVariables()
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
Pipe::setup1Phase()
{
  {
    std::string class_name = "FluidProperties3EqnMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<UserObjectName>("fp") = getParam<UserObjectName>("fp");
    params.set<std::vector<VariableName>>("rhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("rhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("rhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    _sim.addMaterial(class_name, genName(name(), "fp_mat"), params);
  }

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
Pipe::setupHw2Phase()
{
  if (MooseUtils::toLower(_closures_name) == "simple")
  {
    if (_n_heat_transfer_connections > 1)
    {
      // liquid
      {
        const std::string class_name = "WeightedAverageMaterial";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<MaterialPropertyName>("prop_name") =
            FlowModelTwoPhase::HEAT_TRANSFER_COEFFICIENT_WALL_LIQUID;
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<std::vector<MaterialPropertyName>>("values") = _Hw_liquid_names;
        params.set<std::vector<VariableName>>("weights") = _P_hf_names;
        _sim.addMaterial(class_name, genName(name(), "Hw_liquid_material"), params);
      }
      // vapor
      {
        const std::string class_name = "WeightedAverageMaterial";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<MaterialPropertyName>("prop_name") =
            FlowModelTwoPhase::HEAT_TRANSFER_COEFFICIENT_WALL_VAPOR;
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<std::vector<MaterialPropertyName>>("values") = _Hw_vapor_names;
        params.set<std::vector<VariableName>>("weights") = _P_hf_names;
        _sim.addMaterial(class_name, genName(name(), "Hw_vapor_material"), params);
      }
    }
    else if (_n_heat_transfer_connections == 0)
    // weighted average aux would result in division by zero; use zero directly instead
    {
      // liquid
      {
        const std::string class_name = "ConstantMaterial";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<std::string>("property_name") =
            FlowModelTwoPhase::HEAT_TRANSFER_COEFFICIENT_WALL_LIQUID;
        params.set<Real>("value") = 0;
        _sim.addMaterial(class_name, genName(name(), "Hw_liquid_mat"), params);
      }
      // vapor
      {
        const std::string class_name = "ConstantMaterial";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<std::string>("property_name") =
            FlowModelTwoPhase::HEAT_TRANSFER_COEFFICIENT_WALL_VAPOR;
        params.set<Real>("value") = 0;
        _sim.addMaterial(class_name, genName(name(), "Hw_vapor_mat"), params);
      }
    }
  }
  else if (MooseUtils::toLower(_closures_name) == "trace")
  {
    UserObjectName fp_name = getParam<UserObjectName>("fp");
    const std::vector<Real> & alpha_vapor_bounds =
        getParam<std::vector<Real>>("alpha_vapor_bounds");

    ExecFlagEnum lin_execute_on(MooseUtils::getDefaultExecFlagEnum());
    lin_execute_on = {EXEC_LINEAR, EXEC_INITIAL};

    {
      std::string class_name = _app.getWallHeatTransferCoefficent7EqnClassName(_closures_name);
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<MooseEnum>("ht_geom") = getParam<MooseEnum>("heat_transfer_geom");
      if (_HT_geometry == ROD_BUNDLE)
        params.set<Real>("PoD") = _PoD;
      params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
      params.set<std::vector<VariableName>>("arhoA_liquid") = {
          FlowModelTwoPhase::ALPHA_RHO_A_LIQUID};
      params.set<std::vector<VariableName>>("arhoA_vapor") = {FlowModelTwoPhase::ALPHA_RHO_A_VAPOR};
      params.set<std::vector<VariableName>>("D_h") = {FlowModel::HYDRAULIC_DIAMETER};
      params.set<MaterialPropertyName>("alpha_liquid") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
      params.set<MaterialPropertyName>("rho_liquid") = FlowModelTwoPhase::DENSITY_LIQUID;
      params.set<MaterialPropertyName>("vel_liquid") = FlowModelTwoPhase::VELOCITY_LIQUID;
      params.set<MaterialPropertyName>("v_liquid") = FlowModelTwoPhase::SPECIFIC_VOLUME_LIQUID;
      params.set<MaterialPropertyName>("e_liquid") =
          FlowModelTwoPhase::SPECIFIC_INTERNAL_ENERGY_LIQUID;
      params.set<MaterialPropertyName>("T_liquid") = FlowModelTwoPhase::TEMPERATURE_LIQUID;
      params.set<MaterialPropertyName>("p_liquid") = FlowModelTwoPhase::PRESSURE_LIQUID;
      params.set<MaterialPropertyName>("T_wall") = FlowModel::TEMPERATURE_WALL;
      params.set<MaterialPropertyName>("T_sat_liquid") =
          FlowModelTwoPhase::TEMPERATURE_SATURATION_LIQUID;
      params.set<MaterialPropertyName>("alpha_vapor") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
      params.set<MaterialPropertyName>("rho_vapor") = FlowModelTwoPhase::DENSITY_VAPOR;
      params.set<MaterialPropertyName>("vel_vapor") = FlowModelTwoPhase::VELOCITY_VAPOR;
      params.set<MaterialPropertyName>("v_vapor") = FlowModelTwoPhase::SPECIFIC_VOLUME_VAPOR;
      params.set<MaterialPropertyName>("e_vapor") =
          FlowModelTwoPhase::SPECIFIC_INTERNAL_ENERGY_VAPOR;
      params.set<MaterialPropertyName>("T_vapor") = FlowModelTwoPhase::TEMPERATURE_VAPOR;
      params.set<MaterialPropertyName>("p_vapor") = FlowModelTwoPhase::PRESSURE_VAPOR;
      params.set<UserObjectName>("fp") = fp_name;
      if (!_temperature_mode)
        params.set<std::vector<VariableName>>("q_wall") = {FlowModel::HEAT_FLUX_WALL};
      params.set<Real>("alpha_v_min") = alpha_vapor_bounds[0];
      params.set<Real>("alpha_v_max") = alpha_vapor_bounds[1];
      params.set<UserObjectName>("chf_table") = FlowModelTwoPhase::CHF_TABLE;
      params.set<Real>("gravity_magnitude") = _gravity_magnitude;

      _sim.addMaterial(class_name, genName(name(), "wthc_pipe_2phase"), params);
    }
  }
}

void
Pipe::setup2Phase()
{
  std::vector<VariableName> cv_T_wall(1, FlowModel::TEMPERATURE_WALL);
  std::vector<VariableName> cv_temperature_liquid(1, FlowModelTwoPhase::TEMPERATURE_LIQUID);
  std::vector<VariableName> cv_temperature_vapor(1, FlowModelTwoPhase::TEMPERATURE_VAPOR);
  std::vector<VariableName> cv_alpha_liquid(1, FlowModelTwoPhase::VOLUME_FRACTION_LIQUID);
  std::vector<VariableName> cv_alpha_vapor(1, FlowModelTwoPhase::VOLUME_FRACTION_VAPOR);
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  std::vector<VariableName> cv_D_h(1, FlowModel::HYDRAULIC_DIAMETER);
  std::vector<VariableName> cv_P_hf(1, FlowModel::HEAT_FLUX_PERIMETER);

  std::vector<VariableName> cv_vel_liquid(1, FlowModelTwoPhase::VELOCITY_LIQUID);
  std::vector<VariableName> cv_vel_vapor(1, FlowModelTwoPhase::VELOCITY_VAPOR);
  std::vector<VariableName> cv_density_liquid(1, FlowModelTwoPhase::DENSITY_LIQUID);
  std::vector<VariableName> cv_density_vapor(1, FlowModelTwoPhase::DENSITY_VAPOR);
  std::vector<VariableName> cv_v_liquid(1, FlowModelTwoPhase::SPECIFIC_VOLUME_LIQUID);
  std::vector<VariableName> cv_v_vapor(1, FlowModelTwoPhase::SPECIFIC_VOLUME_VAPOR);

  std::vector<VariableName> cv_beta(1, FlowModelTwoPhase::BETA);
  std::vector<VariableName> cv_arhoA_liquid(1, FlowModelTwoPhase::ALPHA_RHO_A_LIQUID);
  std::vector<VariableName> cv_arhouA_liquid(1, FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID);
  std::vector<VariableName> cv_arhoEA_liquid(1, FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID);
  std::vector<VariableName> cv_arhoA_vapor(1, FlowModelTwoPhase::ALPHA_RHO_A_VAPOR);
  std::vector<VariableName> cv_arhouA_vapor(1, FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR);
  std::vector<VariableName> cv_arhoEA_vapor(1, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR);
  std::vector<VariableName> cv_enthalpy_liquid(1,
                                               FlowModelTwoPhase::SPECIFIC_TOTAL_ENTHALPY_LIQUID);
  std::vector<VariableName> cv_enthalpy_vapor(1, FlowModelTwoPhase::SPECIFIC_TOTAL_ENTHALPY_VAPOR);
  std::vector<VariableName> cv_e_liquid(1, FlowModelTwoPhase::SPECIFIC_INTERNAL_ENERGY_LIQUID);
  std::vector<VariableName> cv_e_vapor(1, FlowModelTwoPhase::SPECIFIC_INTERNAL_ENERGY_VAPOR);

  // parameter vectors for loops over phases
  const std::vector<NonlinearVariableName> nonlinear_variable_name{
      FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID, FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};
  const std::vector<bool> is_liquid{true, false};
  const std::vector<std::string> phase_name{"liquid", "vapor"};
  const std::vector<VariableName> arhoA_name{FlowModelTwoPhase::ALPHA_RHO_A_LIQUID,
                                             FlowModelTwoPhase::ALPHA_RHO_A_VAPOR};
  const std::vector<VariableName> arhouA_name{FlowModelTwoPhase::ALPHA_RHOU_A_LIQUID,
                                              FlowModelTwoPhase::ALPHA_RHOU_A_VAPOR};
  const std::vector<VariableName> arhoEA_name{FlowModelTwoPhase::ALPHA_RHOE_A_LIQUID,
                                              FlowModelTwoPhase::ALPHA_RHOE_A_VAPOR};
  const std::vector<MaterialPropertyName> alpha_name{FlowModelTwoPhase::VOLUME_FRACTION_LIQUID,
                                                     FlowModelTwoPhase::VOLUME_FRACTION_VAPOR};
  const std::vector<MaterialPropertyName> T_name{FlowModelTwoPhase::TEMPERATURE_LIQUID,
                                                 FlowModelTwoPhase::TEMPERATURE_VAPOR};

  ExecFlagEnum ts_execute_on(MooseUtils::getDefaultExecFlagEnum());
  ts_execute_on = EXEC_TIMESTEP_BEGIN;

  {
    std::string class_name;
    if (_model_id == THM::FM_TWO_PHASE)
      class_name = "FluidProperties7EqnMaterial";
    else if (_model_id == THM::FM_TWO_PHASE_NCG)
      class_name = "FluidProperties2PhaseNCGMaterial";

    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;

    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("arhoA_liquid") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("arhouA_liquid") = cv_arhouA_liquid;
    params.set<std::vector<VariableName>>("arhoEA_liquid") = cv_arhoEA_liquid;
    params.set<std::vector<VariableName>>("arhoA_vapor") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("arhouA_vapor") = cv_arhouA_vapor;
    params.set<std::vector<VariableName>>("arhoEA_vapor") = cv_arhoEA_vapor;
    params.set<std::vector<VariableName>>("alpha_liquid") = cv_alpha_liquid;
    params.set<std::vector<VariableName>>("alpha_vapor") = cv_alpha_vapor;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<bool>("use_explicit_acoustic_impedance") =
        getParam<bool>("explicit_acoustic_impedance");
    if (_model_id == THM::FM_TWO_PHASE_NCG)
    {
      const FlowModelTwoPhaseNCG & fm = dynamic_cast<const FlowModelTwoPhaseNCG &>(*_flow_model);
      params.set<std::vector<VariableName>>("aXrhoA_vapor") = fm.getNCGSolutionVars();
    }
    _sim.addMaterial(class_name, Component::genName(name(), "fluid_prop_uv_mat"), params);
  }

  setupVolumeFraction();

  setupHw2Phase();

  addFormLossObjects();

  {
    const std::vector<Real> & alpha_vapor_bounds =
        getParam<std::vector<Real>>("alpha_vapor_bounds");
    std::string class_name = _app.getFlowRegimeMapMaterialClassName(_closures_name);
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<UserObjectName>("fp") = _fp_name;
    params.set<Real>("gravity_angle") = _gravity_angle;
    params.set<bool>("horizontal") = _is_horizontal;
    params.set<std::vector<VariableName>>("A") = cv_area;
    params.set<std::vector<VariableName>>("beta") = cv_beta;
    params.set<std::vector<VariableName>>("arhoA_liquid") = cv_arhoA_liquid;
    params.set<std::vector<VariableName>>("arhoA_vapor") = cv_arhoA_vapor;
    params.set<std::vector<VariableName>>("alpha_liquid") = cv_alpha_liquid;
    params.set<std::vector<VariableName>>("alpha_vapor") = cv_alpha_vapor;
    params.set<std::vector<VariableName>>("vel_liquid") = cv_vel_liquid;
    params.set<std::vector<VariableName>>("vel_vapor") = cv_vel_vapor;
    params.set<std::vector<VariableName>>("rho_liquid") = cv_density_liquid;
    params.set<std::vector<VariableName>>("rho_vapor") = cv_density_vapor;
    params.set<std::vector<VariableName>>("v_liquid") = cv_v_liquid;
    params.set<std::vector<VariableName>>("v_vapor") = cv_v_vapor;
    params.set<std::vector<VariableName>>("e_liquid") = cv_e_liquid;
    params.set<std::vector<VariableName>>("e_vapor") = cv_e_vapor;
    params.set<std::vector<VariableName>>("D_h") = cv_D_h;

    params.set<MaterialPropertyName>("cp_liquid") =
        FlowModelTwoPhase::SPECIFIC_HEAT_CONSTANT_PRESSURE_LIQUID;
    params.set<MaterialPropertyName>("cp_vapor") =
        FlowModelTwoPhase::SPECIFIC_HEAT_CONSTANT_PRESSURE_VAPOR;
    params.set<MaterialPropertyName>("k_liquid") = FlowModelTwoPhase::THERMAL_CONDUCTIVITY_LIQUID;
    params.set<MaterialPropertyName>("k_vapor") = FlowModelTwoPhase::THERMAL_CONDUCTIVITY_VAPOR;
    params.set<MaterialPropertyName>("p_liquid") = FlowModelTwoPhase::PRESSURE_LIQUID;
    params.set<MaterialPropertyName>("p_vapor") = FlowModelTwoPhase::PRESSURE_VAPOR;
    params.set<MaterialPropertyName>("T_liquid") = FlowModelTwoPhase::TEMPERATURE_LIQUID;
    params.set<MaterialPropertyName>("T_vapor") = FlowModelTwoPhase::TEMPERATURE_VAPOR;
    params.set<MaterialPropertyName>("mu_liquid") = FlowModelTwoPhase::DYNAMIC_VISCOSITY_LIQUID;
    params.set<MaterialPropertyName>("mu_vapor") = FlowModelTwoPhase::DYNAMIC_VISCOSITY_VAPOR;
    params.set<MaterialPropertyName>("surface_tension") = FlowModelTwoPhase::SURFACE_TENSION;
    params.set<MaterialPropertyName>("h_liquid") = FlowModelTwoPhase::SPECIFIC_ENTHALPY_LIQUID;
    params.set<MaterialPropertyName>("h_vapor") = FlowModelTwoPhase::SPECIFIC_ENTHALPY_VAPOR;

    if (!_temperature_mode)
      params.set<std::vector<VariableName>>("q_wall") = {FlowModel::HEAT_FLUX_WALL};
    params.set<MooseEnum>("ht_geom") = getParam<MooseEnum>("heat_transfer_geom");
    params.set<Real>("alpha_v_min") = alpha_vapor_bounds[0];
    params.set<Real>("alpha_v_max") = alpha_vapor_bounds[1];
    params.set<UserObjectName>("chf_table") = FlowModelTwoPhase::CHF_TABLE;
    /// The following is a placeholder for now; see issue #633
    const bool fp_supports_phase_change = true;
    params.set<bool>("mass_transfer") =
        fp_supports_phase_change && getParam<bool>("wall_mass_transfer");

    params.set<Real>("gravity_magnitude") = _gravity_magnitude;

    if (_HT_geometry == ROD_BUNDLE)
      params.set<Real>("PoD") = _PoD;
    _sim.addMaterial(class_name, genName(name(), "flow_regime_mat"), params);
  }

  if (!_stabilization_uo_name.empty())
  {
    InputParameters pars = emptyInputParameters();
    pars.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    pars.set<Component *>("component") = _parent == nullptr ? this : _parent;
    pars.set<UserObjectName>("fp") = getParam<UserObjectName>("fp");

    const StabilizationSettings & stabilization =
        _sim.getUserObject<StabilizationSettings>(_stabilization_uo_name);
    stabilization.addMooseObjects(*_flow_model, pars);
  }
}

void
Pipe::setupVolumeFraction()
{
  const FlowModelTwoPhase & fm = dynamic_cast<const FlowModelTwoPhase &>(*getFlowModel());
  bool phase_interaction = fm.getPhaseInteraction();

  // materials
  {
    const std::string class_name = "VolumeFractionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    _sim.addMaterial(class_name, genName(name(), class_name), params);
  }

  // aux kernels
  {
    std::string class_name = "VolumeFractionLiquidAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = FlowModelTwoPhase::VOLUME_FRACTION_LIQUID;
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<VariableName>>("beta") = {FlowModelTwoPhase::BETA};
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    _sim.addAuxKernel(class_name, genName(name(), "alpha_liquid"), params);
  }
  {
    std::string class_name = "VolumeFractionAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = FlowModelTwoPhase::VOLUME_FRACTION_VAPOR;
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<std::vector<VariableName>>("opposite_volume_fraction") = {
        FlowModelTwoPhase::VOLUME_FRACTION_LIQUID};
    _sim.addAuxKernel(class_name, genName(name(), "alpha_vapor"), params);
  }

  // Add aux for beta if there is no phase interaction
  if (!phase_interaction)
  {
    const std::string class_name = "RemappedLiquidVolumeFractionAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = FlowModelTwoPhase::BETA;
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<FunctionName>("alpha_vapor") = getVariableFn("initial_alpha_vapor");
    params.set<UserObjectName>("vfm") = FlowModelTwoPhase::VOLUME_FRACTION_MAPPER;
    _sim.addAuxKernel(class_name, genName(name(), class_name), params);
  }
}

void
Pipe::setupDh()
{
  std::vector<VariableName> cv_area(1, FlowModel::AREA);
  ExecFlagEnum ts_execute_on(MooseUtils::getDefaultExecFlagEnum());
  ts_execute_on = {EXEC_TIMESTEP_BEGIN, EXEC_INITIAL};

  std::string nm = genName(name(), "D_h_auxkernel");

  if (_has_Dh)
  {
    std::string class_name = "FunctionAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = FlowModel::HYDRAULIC_DIAMETER;
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<FunctionName>("function") = _Dh_function;
    params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
    _sim.addAuxKernel(class_name, nm, params);

    makeFunctionControllableIfConstant(_Dh_function, "D_h");
  }
  else
  {
    std::string class_name = "HydraulicDiameterCircularAux";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<AuxVariableName>("variable") = FlowModel::HYDRAULIC_DIAMETER;
    params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
    params.set<ExecFlagEnum>("execute_on") = ts_execute_on;
    params.set<std::vector<VariableName>>("A") = cv_area;
    _sim.addAuxKernel(class_name, nm, params);
  }
}

void
Pipe::addFormLossObjects()
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
Pipe::addCommonObjects()
{
  // Wall friction, Dh, and Ph are common objects
  std::vector<VariableName> cv_rho(1, FlowModelSinglePhase::DENSITY);
  std::vector<VariableName> cv_rhou(1, FlowModelSinglePhase::MOMENTUM_DENSITY);
  std::vector<VariableName> cv_rhoE(1, FlowModelSinglePhase::TOTAL_ENERGY_DENSITY);
  std::vector<VariableName> cv_D_h(1, FlowModelSinglePhase::HYDRAULIC_DIAMETER);

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
      const std::string class_name = "WeightedAverageAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = FlowModel::HEAT_FLUX_WALL;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<std::vector<VariableName>>("values") = _q_wall_names;
      params.set<std::vector<VariableName>>("weights") = _P_hf_names;
      params.set<ExecFlagEnum>("execute_on") = execute_on_initial_linear;
      _sim.addAuxKernel(class_name, genName(name(), "q_wall_auxkernel"), params);
    }
    else if (_n_heat_transfer_connections == 0)
    {
      const std::string class_name = "ConstantAux";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<AuxVariableName>("variable") = FlowModel::HEAT_FLUX_WALL;
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<Real>("value") = 0;
      params.set<ExecFlagEnum>("execute_on") = execute_on_initial_linear;
      _sim.addAuxKernel(class_name, genName(name(), "q_wall_auxkernel"), params);
    }
  }
}

void
Pipe::addMooseObjects()
{
  addWallTemperatureObjects();
  addCommonObjects();

  _flow_model->addMooseObjects();
  _closures->addMooseObjects(*this);

  if (_model_id == THM::FM_SINGLE_PHASE)
    setup1Phase();
  else if (_model_id == THM::FM_TWO_PHASE || _model_id == THM::FM_TWO_PHASE_NCG)
    setup2Phase();
}

void
Pipe::addHeatTransferName(const std::string & name) const
{
  _heat_transfer_names.push_back(name);
  _n_heat_transfer_connections++;
}

void
Pipe::getHeatTransferVariableNames()
{
  for (unsigned int i = 0; i < _n_heat_transfer_connections; i++)
  {
    const HeatTransferBase & heat_transfer =
        getComponentByName<HeatTransferBase>(_heat_transfer_names[i]);

    _Hw_1phase_names.push_back(heat_transfer.getWallHeatTransferCoefficient1PhaseName());
    _Hw_liquid_names.push_back(heat_transfer.getWallHeatTransferCoefficientLiquidName());
    _Hw_vapor_names.push_back(heat_transfer.getWallHeatTransferCoefficientVaporName());
    _P_hf_names.push_back(heat_transfer.getHeatedPerimeterName());
    _T_wall_names.push_back(heat_transfer.getWallTemperatureName());
    _q_wall_names.push_back(heat_transfer.getWallHeatFluxName());
  }
}

std::string
Pipe::getHeatTransferNamesSuffix(const std::string & ht_name) const
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

void
Pipe::addWallTemperatureObjects()
{
  ExecFlagEnum execute_on_initial_linear(MooseUtils::getDefaultExecFlagEnum());
  execute_on_initial_linear = {EXEC_INITIAL, EXEC_LINEAR};

  if (_temperature_mode)
  {
    if (_n_heat_transfer_connections > 1)
    {
      // use weighted average wall temperature aux kernel
      if (_model_id == THM::FM_SINGLE_PHASE)
      {
        const std::string class_name = "AverageWallTemperature3EqnMaterial";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<std::vector<VariableName>>("T_wall_sources") = _T_wall_names;
        params.set<std::vector<MaterialPropertyName>>("Hw_sources") = _Hw_1phase_names;
        params.set<std::vector<VariableName>>("P_hf_sources") = _P_hf_names;
        params.set<std::vector<VariableName>>("P_hf_total") = {FlowModel::HEAT_FLUX_PERIMETER};
        params.set<MaterialPropertyName>("Hw_average") =
            FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL;
        params.set<std::vector<VariableName>>("T_fluid") = {FlowModelSinglePhase::TEMPERATURE};
        _sim.addMaterial(class_name, genName(name(), "avg_T_wall_mat"), params);
      }
      else if (_model_id == THM::FM_TWO_PHASE || _model_id == THM::FM_TWO_PHASE_NCG)
      {
        const std::string class_name = "AverageWallTemperature7EqnMaterial";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<std::vector<VariableName>>("T_wall_sources") = _T_wall_names;
        params.set<std::vector<MaterialPropertyName>>("Hw_liquid_sources") = _Hw_liquid_names;
        params.set<std::vector<MaterialPropertyName>>("Hw_vapor_sources") = _Hw_vapor_names;
        params.set<std::vector<VariableName>>("P_hf_sources") = _P_hf_names;
        params.set<MaterialPropertyName>("kappa_liquid") =
            FlowModelTwoPhase::HEAT_FLUX_PARTITIONING_LIQUID;
        params.set<std::vector<VariableName>>("P_hf_total") = {FlowModel::HEAT_FLUX_PERIMETER};
        params.set<MaterialPropertyName>("Hw_liquid_average") =
            FlowModelTwoPhase::HEAT_TRANSFER_COEFFICIENT_WALL_LIQUID;
        params.set<MaterialPropertyName>("Hw_vapor_average") =
            FlowModelTwoPhase::HEAT_TRANSFER_COEFFICIENT_WALL_VAPOR;
        params.set<std::vector<VariableName>>("T_liquid") = {FlowModelTwoPhase::TEMPERATURE_LIQUID};
        params.set<std::vector<VariableName>>("T_vapor") = {FlowModelTwoPhase::TEMPERATURE_VAPOR};
        _sim.addMaterial(class_name, genName(name(), "avg_T_wall_mat"), params);
      }
    }
    else
    {
      // In temperature mode T_wall is prescribed via nodal aux variable, so we need to propagate
      // those values into the material properties, so that RELAP-7 can use them
      const std::string class_name = "CoupledVariableValueMaterial";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
      params.set<MaterialPropertyName>("prop_name") = {FlowModel::TEMPERATURE_WALL};
      params.set<std::vector<VariableName>>("coupled_variable") = {FlowModel::TEMPERATURE_WALL};
      _sim.addMaterial(class_name, genName(name(), "T_wall_var_material"), params);
    }
  }
  else
  {
    if (_model_id == THM::FM_SINGLE_PHASE)
    {
      if (MooseUtils::toLower(_closures_name) == "simple")
      {
        const std::string class_name = "TemperatureWall3EqnMaterial";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
        params.set<std::vector<VariableName>>("q_wall") = {FlowModel::HEAT_FLUX_WALL};
        params.set<MaterialPropertyName>("Hw") =
            FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL;
        _sim.addMaterial(class_name, genName(name(), "T_wall_material"), params);
      }
    }
    else if (_model_id == THM::FM_TWO_PHASE || _model_id == THM::FM_TWO_PHASE_NCG)
    {
      if (MooseUtils::toLower(_closures_name) == "simple")
      {
        const std::string class_name = "TemperatureWall7EqnMaterial";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<std::vector<SubdomainName>>("block") = getSubdomainNames();
        params.set<std::vector<VariableName>>("q_wall") = {FlowModel::HEAT_FLUX_WALL};
        params.set<MaterialPropertyName>("kappa_liquid") =
            FlowModelTwoPhase::HEAT_FLUX_PARTITIONING_LIQUID;
        params.set<MaterialPropertyName>("Hw_liquid") =
            FlowModelTwoPhase::HEAT_TRANSFER_COEFFICIENT_WALL_LIQUID;
        params.set<MaterialPropertyName>("Hw_vapor") =
            FlowModelTwoPhase::HEAT_TRANSFER_COEFFICIENT_WALL_VAPOR;
        params.set<MaterialPropertyName>("T_liquid") = FlowModelTwoPhase::TEMPERATURE_LIQUID;
        params.set<MaterialPropertyName>("T_vapor") = FlowModelTwoPhase::TEMPERATURE_VAPOR;
        _sim.addMaterial(class_name, genName(name(), "T_wall_material"), params);
      }
    }
  }
}

const std::vector<unsigned int> &
Pipe::getNodeIDs() const
{
  checkSetupStatus(MESH_PREPARED);

  return _node_ids;
}

const std::vector<unsigned int> &
Pipe::getElementIDs() const
{
  checkSetupStatus(MESH_PREPARED);

  return _elem_ids;
}

unsigned int
Pipe::getNodesetID() const
{
  checkSetupStatus(MESH_PREPARED);

  return _nodeset_id;
}

const BoundaryName &
Pipe::getNodesetName() const
{
  checkSetupStatus(MESH_PREPARED);

  return _nodeset_name;
}

unsigned int
Pipe::getSubdomainID() const
{
  checkSetupStatus(MESH_PREPARED);

  return _subdomain_id;
}
