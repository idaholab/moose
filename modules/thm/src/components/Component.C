#include "Component.h"
#include "Simulation.h"
#include "MooseApp.h"
#include "ConstantFunction.h"
#include "Numerics.h"

unsigned int Component::subdomain_ids = 0;
unsigned int Component::bc_ids = 0;

template <>
InputParameters
validParams<Component>()
{
  InputParameters params = validParams<RELAP7Object>();
  params.addParam<RealVectorValue>("gravity", RELAP7::default_gravity_vector, "Gravity vector");
  params.addPrivateParam<Simulation *>("_sim");
  params.addPrivateParam<Component *>("_parent", nullptr);
  params.addPrivateParam<std::string>("built_by_action", "add_component");

  params.registerBase("Component");

  return params;
}

/*
 * Component implementation
 */
static unsigned int comp_id = 0;

std::string
Component::genName(const std::string & prefix, unsigned int id, const std::string & suffix)
{
  std::stringstream ss;
  ss << prefix << ":" << id;
  if (!suffix.empty())
    ss << ":" << suffix;
  return ss.str();
}

std::string
Component::genName(const std::string & prefix,
                   unsigned int i,
                   unsigned int j,
                   const std::string & suffix)
{
  std::stringstream ss;
  ss << prefix << ":" << i << ":" << j;
  if (!suffix.empty())
    ss << ":" << suffix;
  return ss.str();
}

std::string
Component::genName(const std::string & prefix,
                   const std::string & middle,
                   const std::string & suffix)
{
  std::stringstream ss;
  ss << prefix << ":" << middle;
  if (!suffix.empty())
    ss << ":" << suffix;
  return ss.str();
}

std::vector<std::string>
Component::split(const std::string & rname)
{
  std::vector<std::string> splitted;
  MooseUtils::tokenize(rname, splitted, 1, ":");

  std::string section_name("");
  for (unsigned int i = 0; i < splitted.size() - 1; i++)
  {
    if (i > 0)
      section_name.append(":");
    section_name.append(splitted[i]);
  }
  std::string prop_name = splitted[splitted.size() - 1];

  // construct the 2 element array with section and property name
  std::vector<std::string> result(2);
  result[0] = section_name;
  result[1] = prop_name;

  return result;
}

Component::Component(const InputParameters & parameters)
  : RELAP7Object(parameters),
    _id(comp_id++),
    _parent(getParam<Component *>("_parent")),
    _sim(*getCheckedPointerParam<Simulation *>("_sim")),
    _app(dynamic_cast<RELAP7App &>(MooseObject::_app)),
    _factory(_app.getFactory()),
    _mesh(_sim.mesh()),
    _zero(_sim._zero),
    _component_setup_status(CREATED)
{
}

void
Component::executeInit()
{
  init();
  _component_setup_status = INITIALIZED_PRIMARY;
}

void
Component::executeInitSecondary()
{
  initSecondary();
  _component_setup_status = INITIALIZED_SECONDARY;
}

void
Component::executeCheck()
{
  check();
  _component_setup_status = CHECKED;
}

void
Component::executeSetupMesh()
{
  setupMesh();
  _component_setup_status = MESH_PREPARED;
}

unsigned int
Component::getNextSubdomainId()
{
  unsigned int id = subdomain_ids++;
  return id;
}

unsigned int
Component::getNextBoundaryId()
{
  unsigned int id = bc_ids++;
  return id;
}

void
Component::connectObject(const InputParameters & params,
                         const std::string & rname,
                         const std::string & mooseName,
                         const std::string & name)
{
  connectObject(params, rname, mooseName, name, name);
}

void
Component::connectObject(const InputParameters & params,
                         const std::string & /*rname*/,
                         const std::string & mooseName,
                         const std::string & name,
                         const std::string & par_name)
{
  MooseObjectParameterName alias("component", this->name(), name, "::");
  MooseObjectParameterName par_value(params.get<std::string>("_moose_base"), mooseName, par_name);
  _app.getInputParameterWarehouse().addControllableParameterAlias(alias, par_value);
}

void
Component::checkSetupStatus(const EComponentSetupStatus & status) const
{
  if (_component_setup_status < status)
    mooseError(name(),
               ": The component setup status (",
               _component_setup_status,
               ") is less than the required status (",
               status,
               ")");
}

void
Component::addDependency(const std::string & dependency)
{
  _dependencies.push_back(dependency);
}

void
Component::makeFunctionControllableIfConstant(const FunctionName & fn_name,
                                              const std::string & control_name,
                                              const std::string & param)
{
  Function & fn = _sim.getFunction(fn_name);
  if (dynamic_cast<ConstantFunction *>(&fn) != nullptr)
    connectObject(fn.parameters(), "", fn_name, control_name, param);
}

void
Component::checkComponentExistsByName(const std::string & comp_name) const
{
  if (!_sim.hasComponent(comp_name))
    logError("The component '", comp_name, "' does not exist");
}

void
Component::checkMutuallyExclusiveParameters(const std::vector<std::string> & params) const
{
  unsigned int n_provided_params = 0;
  for (const auto & param : params)
    if (isParamValid(param))
      n_provided_params++;

  if (n_provided_params != 1)
  {
    std::string params_list_string = "{'" + params[0] + "'";
    for (unsigned int i = 1; i < params.size(); ++i)
      params_list_string += ", '" + params[i] + "'";
    params_list_string += "}";

    if (n_provided_params == 0)
      logError("One of the parameters ", params_list_string, " must be provided");
    else
      logError("Only one of the parameters ", params_list_string, " can be provided");
  }
}

void
Component::checkRDGRequiredParameter(const std::string & param) const
{
  if (!_pars.isParamSetByUser(param))
    logError("The parameter '", param, "' must be provided when using rDG");
}

void
Component::check1PhaseRequiredParameter(const std::string & param) const
{
  if (!isParamValid(param))
    logError("The parameter '", param, "' must be provided for single-phase flow");
}

void
Component::check2PhaseRequiredParameter(const std::string & param) const
{
  if (!isParamValid(param))
    logError("The parameter '", param, "' must be provided for two-phase flow");
}

void
Component::check7EqnRequiredParameter(const std::string & param) const
{
  if (!isParamValid(param))
    logError("The parameter '", param, "' must be provided for the 7-equation model");
}

void
Component::logModelNotImplementedError(const RELAP7::FlowModelID & model) const
{
  if (model == RELAP7::FM_SINGLE_PHASE)
    logError("This component is not implemented for single-phase flow");
  else if (model == RELAP7::FM_TWO_PHASE)
    logError("This component is not implemented for two-phase flow");
  else
    logError("This component is not implemented for model type '", model, "'");
}

void
Component::logSpatialDiscretizationNotImplementedError(
    const FlowModel::ESpatialDiscretizationType & spatial_discretization) const
{
  if (spatial_discretization == FlowModel::CG)
    logError("This component is not implemented for CG spatial discretization");
  else if (spatial_discretization == FlowModel::rDG)
    logError("This component is not implemented for rDG spatial discretization");
  else
    logError("This component is not implemented for spatial discretization type '",
             spatial_discretization,
             "'");
}
