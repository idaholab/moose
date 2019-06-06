#include "Simulation.h"

#include "MooseObjectAction.h"
#include "Transient.h"

#include "FlowChannelBase.h"
#include "FlowJunction.h"

#include "FluidProperties.h"
#include "THMControl.h"
#include "TerminateControl.h"
#include "THMMesh.h"

Simulation::Simulation(ActionWarehouse & action_warehouse)
  : LoggingInterface(dynamic_cast<THMApp &>(action_warehouse.mooseApp())),
    _action_warehouse(action_warehouse),
    _fe_problem(nullptr),
    _app(dynamic_cast<THMApp &>(_action_warehouse.mooseApp())),
    _factory(_app.getFactory()),
    _action_factory(_app.getActionFactory()),
    _pars(emptyInputParameters()),
    _flow_fe_type(FEType(CONSTANT, MONOMIAL)),
    _spatial_discretization(FlowModel::rDG),
    _implicit_time_integration(true),
    _zero(0)
{
  {
    InputParameters params = _factory.getValidParams("THMMesh");
    params.set<MooseEnum>("dim") = "3";
    params.set<unsigned int>("patch_size") = 1;
    // We need to go through the Factory to create the THMMesh so
    // that its params object is properly added to the Warehouse.
    _mesh = _factory.create<THMMesh>("THMMesh", "THM:mesh", params);
  }

  // NOTE: when Simulation gets merged in THMProblem, these will reside in its input parameters,
  // so this ugliness will go away
  params().set<std::vector<Real>>("scaling_factor_1phase") = {1., 1., 1.};
  params().set<std::vector<Real>>("scaling_factor_2phase") = {1., 1., 1., 1., 1., 1., 1.};
  params().set<Real>("scaling_factor_temperature") = 1.;
}

Simulation::~Simulation()
{
  for (auto && k : _control_data)
    delete k.second;

  // _mesh is destroyed by MOOSE
}

THMMesh &
Simulation::mesh()
{
  return *_mesh;
}

InputParameters &
Simulation::params()
{
  return _pars;
}

FEProblem &
Simulation::feproblem()
{
  return *_fe_problem;
}

void
Simulation::buildMesh()
{
  if (_components.size() == 0)
    return;

  _mesh->setMeshBase(_mesh->buildMeshBaseObject());

  // build mesh
  for (auto && comp : _components)
    comp->executeSetupMesh();
  // Some components add side sets, some add node sets. Make sure both versions exist
  _mesh->getMesh().get_boundary_info().build_side_list_from_node_list();
  _mesh->prep();

  // store in parser
  _action_warehouse.mesh() = _mesh;
}

void
Simulation::init()
{
  // sort the components using dependency resolver
  DependencyResolver<std::shared_ptr<Component>> dependency_resolver;
  for (const auto & comp : _components)
    for (const auto & dep : comp->getDependencies())
      dependency_resolver.insertDependency(comp, _comp_by_name[dep]);
  std::sort(_components.begin(), _components.end(), dependency_resolver);
}

void
Simulation::initComponents()
{
  // initialize components
  for (auto && comp : _components)
    comp->executeInit();

  // perform secondary initialization, which relies on init() being called
  // already for all components
  for (auto && comp : _components)
    comp->executeInitSecondary();
}

void
Simulation::identifyLoops()
{
  // loop over junctions and boundaries (non-geometrical components)
  for (const auto & component : _components)
  {
    const auto flow_connection =
        MooseSharedNamespace::dynamic_pointer_cast<FlowConnection>(component);
    if (flow_connection)
    {
      // create vector of names of this component and its connected flow channels, and then sort
      // them
      std::vector<std::string> names = flow_connection->getConnectedComponentNames();
      names.push_back(component->name());
      std::sort(names.begin(), names.end());

      // pick first name alphabetically to be the proposed loop name
      std::string proposed_loop_name = names[0];

      for (const std::string & name : names)
      {
        // if the name is not yet in the map
        if (_component_name_to_loop_name.find(name) == _component_name_to_loop_name.end())
          // just add the new map key; nothing else needs updating
          _component_name_to_loop_name[name] = proposed_loop_name;
        else
        {
          // compare to the existing loop name for this component to make sure the
          // proposed loop name is first alphabetically
          const std::string current_loop_name = _component_name_to_loop_name[name];
          // if proposed loop name comes later, change map values of the current
          // loop name to be the proposed name, and then update the proposed name
          // to be the current name
          if (proposed_loop_name > current_loop_name)
          {
            for (auto && entry : _component_name_to_loop_name)
              if (entry.second == proposed_loop_name)
                entry.second = current_loop_name;
            proposed_loop_name = current_loop_name;
          }
          // if proposed loop name comes earlier, change map values of the current
          // loop name to be the proposed name
          else if (proposed_loop_name < current_loop_name)
          {
            for (auto && entry : _component_name_to_loop_name)
              if (entry.second == current_loop_name)
                entry.second = proposed_loop_name;
          }
        }
      }
    }
  }

  // get the list of loops
  std::vector<std::string> loops;
  for (const auto & entry : _component_name_to_loop_name)
    if (std::find(loops.begin(), loops.end(), entry.second) == loops.end())
      loops.push_back(entry.second);

  // fill the map of loop name to model ID
  for (const auto & loop : loops)
  {
    // find a flow channel in this loop and get its model ID
    THM::FlowModelID model_id;
    bool found_model_id = false;
    for (const auto & component : _components)
    {
      const auto flow_chan_base_component =
          MooseSharedNamespace::dynamic_pointer_cast<FlowChannelBase>(component);
      if (flow_chan_base_component && (_component_name_to_loop_name[component->name()] == loop))
      {
        const UserObjectName fp_name = flow_chan_base_component->getFluidPropertiesName();
        const FluidProperties & fp = getUserObject<FluidProperties>(fp_name);
        model_id = _app.getFlowModelID(fp);
        found_model_id = true;
        break;
      }
    }
    // set the value in the map or throw an error
    if (found_model_id)
      _loop_name_to_model_id[loop] = model_id;
    else
      logError("No FlowChannelBase-derived components were found in loop '", loop, "'");
  }
}

void
Simulation::printComponentLoops() const
{
  // get the list of loops
  std::vector<std::string> loops;
  for (auto && entry : _component_name_to_loop_name)
    if (std::find(loops.begin(), loops.end(), entry.second) == loops.end())
      loops.push_back(entry.second);

  // for each loop
  Moose::out << "\nListing of component loops:" << std::endl;
  for (unsigned int i = 0; i < loops.size(); i++)
  {
    Moose::out << "\n  Loop " << i + 1 << ":" << std::endl;

    // print out each component in the loop
    for (auto && entry : _component_name_to_loop_name)
      if (entry.second == loops[i])
        Moose::out << "    " << entry.first << std::endl;
  }
  Moose::out << std::endl;
}

void
Simulation::addIterationCountPostprocessors()
{
  const std::vector<std::string> it_per_step_class_names = {"NumLinearIterations",
                                                            "NumNonlinearIterations"};
  const std::vector<std::string> it_per_step_names = {"num_linear_iterations_per_step",
                                                      "num_nonlinear_iterations_per_step"};
  const std::vector<std::string> total_it_names = {"num_linear_iterations",
                                                   "num_nonlinear_iterations"};

  for (unsigned int i = 0; i < it_per_step_class_names.size(); i++)
  {
    // iterations per time step
    {
      const std::string class_name = "AddPostprocessorAction";
      InputParameters action_params = _action_factory.getValidParams(class_name);
      action_params.set<std::string>("type") = it_per_step_class_names[i];
      auto action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, it_per_step_names[i], action_params));
      _action_warehouse.addActionBlock(action);
    }
    // cumulative iterations
    {
      const std::string class_name = "AddPostprocessorAction";
      InputParameters action_params = _action_factory.getValidParams(class_name);
      action_params.set<std::string>("type") = "CumulativeValuePostprocessor";
      auto action = std::static_pointer_cast<MooseObjectAction>(
          _action_factory.create(class_name, total_it_names[i], action_params));
      action->getObjectParams().set<PostprocessorName>("postprocessor") = it_per_step_names[i];
      _action_warehouse.addActionBlock(action);
    }
  }
}

void
Simulation::addVariable(
    bool nl, const std::string & name, FEType type, unsigned int subdomain_id, Real scaling_factor)
{
  if (_vars.find(name) == _vars.end())
  {
    VariableInfo vi;
    vi._nl = nl;
    vi._type = type;
    vi._subdomain.insert(subdomain_id);
    vi._scaling_factor = scaling_factor;
    _vars[name] = vi;
  }
  else
  {
    VariableInfo & vi = _vars[name];
    if (vi._type != type)
      mooseError(
          "A component is trying to add variable of the same name but with different order/type");
    vi._subdomain.insert(subdomain_id);
  }
}

void
Simulation::addVariable(bool nl,
                        const std::string & name,
                        FEType type,
                        const std::vector<unsigned int> & subdomain_ids,
                        Real scaling_factor /* = 1.*/)
{
  for (auto && sid : subdomain_ids)
    addVariable(nl, name, type, sid, scaling_factor);
}

void
Simulation::addKernel(const std::string & type, const std::string & name, InputParameters params)
{
  _fe_problem->addKernel(type, name, params);
}

void
Simulation::addDGKernel(const std::string & type, const std::string & name, InputParameters params)
{
  _fe_problem->addDGKernel(type, name, params);
}

void
Simulation::addAuxKernel(const std::string & type, const std::string & name, InputParameters params)
{
  _fe_problem->addAuxKernel(type, name, params);
}

void
Simulation::addScalarKernel(const std::string & type,
                            const std::string & name,
                            InputParameters params)
{
  _fe_problem->addScalarKernel(type, name, params);
}

void
Simulation::addAuxScalarKernel(const std::string & type,
                               const std::string & name,
                               InputParameters params)
{
  _fe_problem->addAuxScalarKernel(type, name, params);
}

void
Simulation::addBoundaryCondition(const std::string & type,
                                 const std::string & name,
                                 InputParameters params)
{
  _fe_problem->addBoundaryCondition(type, name, params);
}

void
Simulation::addAuxBoundaryCondition(const std::string & type,
                                    const std::string & name,
                                    InputParameters params)
{
  _fe_problem->addAuxKernel(type, name, params);
}

void
Simulation::addFunction(const std::string & type, const std::string & name, InputParameters params)
{
  _fe_problem->addFunction(type, name, params);
}

void
Simulation::addMaterial(const std::string & type, const std::string & name, InputParameters params)
{
  _fe_problem->addMaterial(type, name, params);
}

void
Simulation::addPostprocessor(const std::string & type,
                             const std::string & name,
                             InputParameters params)
{
  _fe_problem->addPostprocessor(type, name, params);
}

void
Simulation::addVectorPostprocessor(const std::string & type,
                                   const std::string & name,
                                   InputParameters params)
{
  _fe_problem->addVectorPostprocessor(type, name, params);
}

void
Simulation::addConstraint(const std::string & type,
                          const std::string & name,
                          InputParameters params)
{
  _fe_problem->addConstraint(type, name, params);
}

void
Simulation::addUserObject(const std::string & type,
                          const std::string & name,
                          InputParameters params)
{
  _fe_problem->addUserObject(type, name, params);
}

void
Simulation::addTransfer(const std::string & type, const std::string & name, InputParameters params)
{
  _fe_problem->addTransfer(type, name, params);
}

void
Simulation::addControl(const std::string & type, const std::string & name, InputParameters params)
{
  params.addPrivateParam<FEProblemBase *>("_fe_problem_base", _fe_problem);
  std::shared_ptr<Control> control = _factory.create<Control>(type, name, params);
  _fe_problem->getControlWarehouse().addObject(control);
}

Real &
Simulation::getPostprocessorValue(const std::string & name)
{
  return _fe_problem->getPostprocessorValue(name);
}

void
Simulation::addInitialCondition(const std::string & type,
                                const std::string & name,
                                InputParameters params)
{
  if (hasInitialConditionsFromFile())
    return;

  if (_ics.find(name) == _ics.end())
  {
    ICInfo ic(type, params);
    _ics[name] = ic;
  }
  else
    mooseError("Initial condition with name '", name, "' already exists.");
}

void
Simulation::addConstantIC(const std::string & var_name,
                          Real value,
                          const SubdomainName & block_name)
{
  if (hasInitialConditionsFromFile())
    return;

  std::string class_name = "ConstantIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = var_name;
  params.set<Real>("value") = value;
  params.set<std::vector<SubdomainName>>("block") = {block_name};
  addInitialCondition(class_name, Component::genName(var_name, block_name, "ic"), params);
}

void
Simulation::addConstantIC(const std::string & var_name,
                          Real value,
                          const std::vector<SubdomainName> & block_names)
{
  if (hasInitialConditionsFromFile())
    return;

  std::string blk_str = block_names[0];
  for (unsigned int i = 1; i < block_names.size(); i++)
    blk_str += ":" + block_names[i];

  std::string class_name = "ConstantIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = var_name;
  params.set<Real>("value") = value;
  params.set<std::vector<SubdomainName>>("block") = block_names;
  addInitialCondition(class_name, Component::genName(var_name, blk_str, "ic"), params);
}

void
Simulation::addFunctionIC(const std::string & var_name,
                          const std::string & func_name,
                          const SubdomainName & block_name)
{
  if (hasInitialConditionsFromFile())
    return;

  std::string class_name = "FunctionIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = var_name;
  params.set<std::vector<SubdomainName>>("block") = {block_name};
  params.set<FunctionName>("function") = func_name;
  addInitialCondition(class_name, Component::genName(var_name, block_name, "ic"), params);
}

void
Simulation::addFunctionIC(const std::string & var_name,
                          const std::string & func_name,
                          const std::vector<SubdomainName> & block_names)
{
  if (hasInitialConditionsFromFile())
    return;

  std::string blk_str = block_names[0];
  for (unsigned int i = 1; i < block_names.size(); i++)
    blk_str += ":" + block_names[i];

  std::string class_name = "FunctionIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = var_name;
  params.set<std::vector<SubdomainName>>("block") = block_names;
  params.set<FunctionName>("function") = func_name;
  addInitialCondition(class_name, Component::genName(var_name, blk_str, "ic"), params);
}

void
Simulation::addConstantScalarIC(const std::string & var_name, Real value)
{
  if (hasInitialConditionsFromFile())
    return;

  std::string class_name = "ScalarConstantIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = var_name;
  params.set<Real>("value") = value;
  addInitialCondition(class_name, Component::genName(var_name, "ic"), params);
}

void
Simulation::addComponentScalarIC(const std::string & var_name, const std::vector<Real> & value)
{
  if (hasInitialConditionsFromFile())
    return;

  std::string class_name = "ScalarComponentIC";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<VariableName>("variable") = var_name;
  params.set<std::vector<Real>>("values") = value;
  addInitialCondition(class_name, Component::genName(var_name, "ic"), params);
}

void
Simulation::addVariables()
{
  Transient * trex = dynamic_cast<Transient *>(getApp().getExecutioner());
  if (trex)
  {
    Moose::TimeIntegratorType ti_type = trex->getTimeScheme();
    // This is only needed for the listed time integrators that are using the original approach to
    // explicit integration in MOOSE.  Currently, the new time integrators like
    // ActuallyExplicitEuler do not need the implicit flag to be set.
    if (ti_type == Moose::TI_EXPLICIT_TVD_RK_2 || ti_type == Moose::TI_EXPLICIT_MIDPOINT ||
        ti_type == Moose::TI_EXPLICIT_EULER)
      _implicit_time_integration = false;
  }

  if (_components.size() == 0)
    return;

  // let all components add their variables
  for (auto && comp : _components)
    comp->addVariables();

  // pass the variables to MOOSE
  for (auto && v : _vars)
  {
    std::string name = v.first;
    if (name.size() > THM::MAX_VARIABLE_LENGTH)
      mooseError(
          "Variable name '", name, "' is too long. The limit is ", THM::MAX_VARIABLE_LENGTH, ".");

    VariableInfo & vi = v.second;

    std::set<subdomain_id_type> * subdomain = nullptr;
    if (!vi._subdomain.empty())
      subdomain = &vi._subdomain;

    if (vi._type.family != SCALAR)
    {
      if (vi._nl)
        _fe_problem->addVariable(name, vi._type, vi._scaling_factor, subdomain);
      else
        _fe_problem->addAuxVariable(name, vi._type, subdomain);
    }
  }

  // pass the scalar variables to MOOSE
  for (auto && v : _vars)
  {
    const std::string & name = v.first;
    if (name.size() > THM::MAX_VARIABLE_LENGTH)
      mooseError(
          "Variable name '", name, "' is too long. The limit is ", THM::MAX_VARIABLE_LENGTH, ".");
    const VariableInfo & vi = v.second;

    const std::set<subdomain_id_type> * subdomain = nullptr;
    if (!vi._subdomain.empty())
      subdomain = &vi._subdomain;

    if (vi._type.family == SCALAR)
    {
      if (vi._nl)
        _fe_problem->addScalarVariable(name, vi._type.order, vi._scaling_factor, subdomain);
      else
        _fe_problem->addAuxScalarVariable(name, vi._type.order, vi._scaling_factor, subdomain);
    }
  }

  if (hasInitialConditionsFromFile())
    setupInitialConditionsFromFile();
  else
    setupInitialConditions();
}

void
Simulation::setupInitialConditionsFromFile()
{
  const UserObjectName suo_name = Component::genName("thm", "suo");
  {
    const std::string class_name = "SolutionUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MeshFileName>("mesh") = getParam<FileName>("initial_from_file");
    params.set<std::string>("timestep") = getParam<std::string>("initial_from_file_timestep");
    _fe_problem->addUserObject(class_name, suo_name, params);
  }

  for (auto && v : _vars)
  {
    const std::string & var_name = v.first;
    VariableInfo & vi = v.second;

    if (vi._type.family == SCALAR)
    {
      std::string class_name = "ScalarSolutionInitialCondition";
      InputParameters params = _factory.getValidParams(class_name);
      params.set<VariableName>("variable") = var_name;
      params.set<VariableName>("from_variable") = var_name;
      params.set<UserObjectName>("solution_uo") = suo_name;
      _fe_problem->addInitialCondition(class_name, Component::genName(var_name, "ic"), params);
    }
    else
    {
      if (vi._subdomain.size() > 0)
      {
        for (auto & sid : vi._subdomain)
        {
          SubdomainName block_name = _mesh->getSubdomainName(sid);

          std::string class_name = "SolutionInitialCondition";
          InputParameters params = _factory.getValidParams(class_name);
          params.set<VariableName>("variable") = var_name;
          params.set<VariableName>("from_variable") = var_name;
          params.set<UserObjectName>("solution_uo") = suo_name;
          params.set<std::vector<SubdomainName>>("block") = {block_name};
          _fe_problem->addInitialCondition(
              class_name, Component::genName(var_name, block_name, "ic"), params);
        }
      }
      else
      {
        std::string class_name = "SolutionInitialCondition";
        InputParameters params = _factory.getValidParams(class_name);
        params.set<VariableName>("variable") = var_name;
        params.set<VariableName>("from_variable") = var_name;
        params.set<UserObjectName>("solution_uo") = suo_name;
        _fe_problem->addInitialCondition(class_name, Component::genName(var_name, "ic"), params);
      }
    }
  }
}

void
Simulation::setupInitialConditions()
{
  for (auto && i : _ics)
  {
    const std::string & name = i.first;
    ICInfo & ic = i.second;
    _fe_problem->addInitialCondition(ic._type, name, ic._params);
  }
}

bool
Simulation::hasFunction(const std::string & name, THREAD_ID tid)
{
  return feproblem().hasFunction(name, tid);
}

Function &
Simulation::getFunction(const std::string & name, THREAD_ID tid)
{
  return feproblem().getFunction(name, tid);
}

void
Simulation::addComponentPhysics()
{
  // let all components add their own
  for (auto && comp : _components)
    comp->addMooseObjects();
}

void
Simulation::build()
{
}

void
Simulation::ghostElements()
{
  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem = _mesh->nodeToElemMap();

  for (auto && comp : _components)
  {
    auto jct = dynamic_cast<FlowJunction *>(comp.get());
    if (jct != nullptr)
    {
      const std::vector<dof_id_type> & node_ids = jct->getNodeIDs();

      // go over nodes the junction is connected to
      for (const auto & nid : node_ids)
      {
        const auto & it = node_to_elem.find(nid);
        if (it == node_to_elem.end())
          mooseError("Failed to find Node ", nid, "in the Mesh!");
        // ghost all elements connected to the nodes
        const std::vector<dof_id_type> & elems = it->second;
        for (const auto & elem_it : elems)
          _fe_problem->addGhostedElem(elem_it);
      }
    }
  }
}

void
Simulation::setupCoordinateSystem()
{
  MultiMooseEnum coord_types("XYZ RZ RSPHERICAL");
  std::vector<SubdomainName> blocks;

  for (auto && comp : _components)
  {
    GeometricalComponent * gc = dynamic_cast<GeometricalComponent *>(comp.get());
    if (gc != NULL && gc->parent() == nullptr)
    {
      const std::vector<unsigned int> & subdomains = gc->getSubdomainIds();
      const std::vector<Moose::CoordinateSystemType> & coord_sys = gc->getCoordSysTypes();

      for (unsigned int i = 0; i < subdomains.size(); i++)
      {
        blocks.push_back(Moose::stringify<unsigned int>(subdomains[i]));
        // coord_types.push_back("XYZ");
        coord_types.push_back(coord_sys[i] == Moose::COORD_RZ ? "RZ" : "XYZ");
      }
    }
  }
  _fe_problem->setCoordSystem(blocks, coord_types);

  // RZ geometries are always aligned with x-axis
  MooseEnum rz_coord_axis("X=0 Y=1", "X");
  _fe_problem->setAxisymmetricCoordAxis(rz_coord_axis);
}

void
Simulation::setupMesh()
{
  _fe_problem = dynamic_cast<FEProblem *>(_action_warehouse.problemBase().get());
  if (_fe_problem == nullptr)
    mooseError("You need to be running with FEProblem derived class.");

  if (_components.size() == 0)
    return;

  setupCoordinateSystem();
  ghostElements();
}

void
Simulation::integrityCheck() const
{
  if (_components.size() == 0)
    return;

  // go over components and put flow channels into one "bucket"
  std::vector<Component *> flow_channels;
  for (auto && comp : _components)
  {
    auto flow_channel = dynamic_cast<FlowChannelBase *>(comp.get());
    if (flow_channel != nullptr)
      flow_channels.push_back(flow_channel);
  }

  // initialize number of connected flow channel inlets and outlets to zero
  std::map<std::string, unsigned int> flow_channel_inlets;
  std::map<std::string, unsigned int> flow_channel_outlets;
  for (auto && comp : flow_channels)
  {
    flow_channel_inlets[comp->name()] = 0;
    flow_channel_outlets[comp->name()] = 0;
  }

  // mark connections of any FlowConnection components
  for (const auto & comp : _components)
  {
    auto pc_comp = dynamic_cast<FlowConnection *>(comp.get());
    if (pc_comp != nullptr)
    {
      for (const auto & connection : pc_comp->getConnections())
      {
        if (connection._end_type == FlowConnection::IN)
          flow_channel_inlets[connection._geometrical_component_name]++;
        else if (connection._end_type == FlowConnection::OUT)
          flow_channel_outlets[connection._geometrical_component_name]++;
      }
    }
  }

  // finally, check that each flow channel has exactly one input and one output
  for (auto && comp : flow_channels)
  {
    if (flow_channel_inlets[comp->name()] == 0)
      logError("Component '", comp->name(), "' does not have connected inlet.");
    else if (flow_channel_inlets[comp->name()] > 1)
      logError("Multiple inlets specified for component '", comp->name(), "'.");

    if (flow_channel_outlets[comp->name()] == 0)
      logError("Component '", comp->name(), "' does not have connected outlet.");
    else if (flow_channel_outlets[comp->name()] > 1)
      logError("Multiple outlets specified for component '", comp->name(), "'.");
  }

  // let components check themselves
  for (auto && comp : _components)
    comp->executeCheck();

  if (_app.log().getNumberOfErrors() > 0)
  {
    Moose::err << COLOR_RED
               << "Execution stopped, the following problems were found:" << COLOR_DEFAULT
               << std::endl
               << std::endl;
    _app.log().print();
    Moose::err << std::endl;
    MOOSE_ABORT;
  }

  if (_app.log().getNumberOfWarnings() > 0)
  {
    _app.log().print();
    Moose::err << std::endl;
  }
}

void
Simulation::controlDataIntegrityCheck()
{
  // check that control data are consistent
  for (auto && i : _control_data)
  {
    if (!i.second->getDeclared())
      logError("Control data '",
               i.first,
               "' was requested, but was not declared by any active control object.");
  }

  if (_app.log().getNumberOfErrors() > 0)
  {
    Moose::err << COLOR_RED
               << "Execution stopped, the following problems were found:" << COLOR_DEFAULT
               << std::endl
               << std::endl;
    _app.log().print();
    Moose::err << std::endl;
    MOOSE_ABORT;
  }

  auto & ctrl_wh = _fe_problem->getControlWarehouse()[EXEC_TIMESTEP_BEGIN];

  // initialize THM control objects
  for (auto && i : ctrl_wh.getObjects())
  {
    THMControl * ctrl = dynamic_cast<THMControl *>(i.get());
    if (ctrl != nullptr)
      ctrl->init();
  }

  for (auto && i : ctrl_wh.getObjects())
  {
    THMControl * ctrl = dynamic_cast<THMControl *>(i.get());
    // if it is a THM control
    if (ctrl != nullptr)
    {
      // get its dependencies on control data
      auto & cd_deps = ctrl->getControlDataDependencies();
      for (auto && cd_name : cd_deps)
      {
        ControlDataValue * cdv = _control_data[cd_name];
        // find out which control object built the control data
        std::string dep_name = cdv->getControl().name();
        auto & deps = ctrl->getDependencies();
        // and if it is not in its dependency list, add it
        auto it = std::find(deps.begin(), deps.end(), dep_name);
        if (it == deps.end())
          deps.push_back(dep_name);
      }
    }
  }

  // Find all `TerminateControl`s and all their dependencies. Then add those
  // objects into TIMESTEP_END control warehouse
  MooseObjectWarehouse<Control> & ctrl_wh_tse =
      _fe_problem->getControlWarehouse()[EXEC_TIMESTEP_END];
  for (auto && i : ctrl_wh.getObjects())
  {
    if (TerminateControl * ctrl = dynamic_cast<TerminateControl *>(i.get()))
    {
      std::list<THMControl *> l;
      l.push_back(ctrl);
      while (l.size() > 0)
      {
        THMControl * ctrl = l.front();
        auto & cd_deps = ctrl->getControlDataDependencies();
        for (auto && cd_name : cd_deps)
        {
          ControlDataValue * cdv = _control_data[cd_name];
          l.push_back(&cdv->getControl());
        }
        ctrl_wh_tse.addObject(ctrl_wh.getObject(ctrl->name()));
        l.pop_front();
      }
    }
  }
}

void
Simulation::run()
{
}

void
Simulation::addComponent(const std::string & type, const std::string & name, InputParameters params)
{
  params.set<Simulation *>("_sim") = this;
  std::shared_ptr<Component> comp = _factory.create<Component>(type, name, params);
  if (_comp_by_name.find(name) == _comp_by_name.end())
    _comp_by_name[name] = comp;
  else
    logError("Component with name '", name, "' already exists");
  _components.push_back(comp);
}

bool
Simulation::hasComponent(const std::string & name) const
{
  auto it = _comp_by_name.find(name);
  return (it != _comp_by_name.end());
}

void
Simulation::addFileOutputter(const std::string & name)
{
  _outputters_all.push_back(name);
  _outputters_file.push_back(name);
}

void
Simulation::addScreenOutputter(const std::string & name)
{
  _outputters_all.push_back(name);
  _outputters_screen.push_back(name);
}

std::vector<OutputName>
Simulation::getOutputsVector(const std::string & key) const
{
  std::string key_lowercase = key;
  std::transform(key_lowercase.begin(), key_lowercase.end(), key_lowercase.begin(), ::tolower);

  std::vector<OutputName> outputs;
  if (key_lowercase == "none")
    outputs.push_back("none"); // provide non-existent name, so it does not get printed out
  else if (key_lowercase == "screen")
    outputs = _outputters_screen;
  else if (key_lowercase == "file")
    outputs = _outputters_file;
  else if (key_lowercase == "both")
    outputs = _outputters_all;
  else
    mooseError("The outputs vector key '" + key_lowercase + "' is invalid");

  return outputs;
}

bool
Simulation::hasInitialConditionsFromFile()
{
  return _pars.isParamValid("initial_from_file");
}
