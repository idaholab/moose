#include "Component.h"
#include "ConstantFunction.h"
#include "Numerics.h"

template <>
InputParameters
validParams<Component>()
{
  InputParameters params = validParams<THMObject>();
  params.addParam<RealVectorValue>(
      "gravity_vector", THM::default_gravity_vector, "Gravitational acceleration vector");
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

Component::Component(const InputParameters & parameters)
  : THMObject(parameters),
    LoggingInterface(dynamic_cast<THMApp &>(MooseObject::_app)),
    NamingInterface(),

    _gravity_vector(getParam<RealVectorValue>("gravity_vector")),
    _gravity_magnitude(_gravity_vector.norm()),
    _gravity_is_zero(MooseUtils::absoluteFuzzyEqual(_gravity_magnitude, 0.0)),
    _gravity_direction(_gravity_is_zero ? RealVectorValue(0.0, 0.0, 0.0) : _gravity_vector.unit()),

    _id(comp_id++),
    _parent(getParam<Component *>("_parent")),
    _sim(*getCheckedPointerParam<Simulation *>("_sim")),
    _app(dynamic_cast<THMApp &>(MooseObject::_app)),
    _factory(_app.getFactory()),
    _mesh(_sim.mesh()),
    _zero(_sim._zero),
    _component_setup_status(CREATED)
{
}

const std::string &
Component::cname() const
{
  if (_parent)
    return _parent->cname();
  else
    return name();
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
Component::executeCheck() const
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

void
Component::connectObject(const InputParameters & params,
                         const std::string & mooseName,
                         const std::string & name) const
{
  connectObject(params, mooseName, name, name);
}

void
Component::connectObject(const InputParameters & params,
                         const std::string & mooseName,
                         const std::string & name,
                         const std::string & par_name) const
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
                                              const std::string & param) const
{
  const Function & fn = _sim.getFunction(fn_name);
  if (dynamic_cast<const ConstantFunction *>(&fn) != nullptr)
    connectObject(fn.parameters(), fn_name, control_name, param);
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
Component::check7EqnNCGRequiredParameter(const std::string & param) const
{
  if (!isParamValid(param))
    logError("The parameter '",
             param,
             "' must be provided for the 7-equation model with non-condensable gas");
}

void
Component::logModelNotImplementedError(const THM::FlowModelID & model) const
{
  if (model == THM::FM_SINGLE_PHASE)
    logError("This component is not implemented for single-phase flow");
  else if (model == THM::FM_TWO_PHASE)
    logError("This component is not implemented for two-phase flow");
  else if (model == THM::FM_TWO_PHASE_NCG)
    logError("This component is not implemented for two-phase flow with non-condensable gases");
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
