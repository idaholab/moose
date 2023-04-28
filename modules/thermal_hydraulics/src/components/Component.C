//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Component.h"
#include "THMMesh.h"
#include "ThermalHydraulicsApp.h"
#include "ConstantFunction.h"
#include "Numerics.h"
#include "RelationshipManager.h"

InputParameters
Component::validParams()
{
  InputParameters params = THMObject::validParams();
  params.addPrivateParam<THMProblem *>("_thm_problem");
  params.addPrivateParam<Component *>("_parent", nullptr);
  params.addPrivateParam<std::string>("built_by_action", "add_component");

  params.registerBase("Component");

  return params;
}

/*
 * Component implementation
 */

Component::Component(const InputParameters & parameters)
  : THMObject(parameters),
    LoggingInterface(getCheckedPointerParam<THMProblem *>("_thm_problem")->log()),
    NamingInterface(),

    _parent(getParam<Component *>("_parent")),
    _sim(*getCheckedPointerParam<THMProblem *>("_thm_problem")),
    _factory(_app.getFactory()),
    _zero(_sim._real_zero[0]),
    _mesh(static_cast<THMMesh &>(_sim.mesh())),
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

THMMesh &
Component::mesh()
{
  if (_component_setup_status >= MESH_PREPARED)
    mooseError(
        "A non-const reference to the THM mesh cannot be obtained after mesh setup is complete.");
  else
    return _mesh;
}

void
Component::executePreSetupMesh()
{
  preSetupMesh();
  _component_setup_status = PRE_SETUP_MESH_COMPLETED;
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

THMProblem &
Component::getTHMProblem() const
{
  return _sim;
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
Component::addRelationshipManagersFromParameters(const InputParameters & moose_object_pars)
{
  const auto & buildable_types = moose_object_pars.getBuildableRelationshipManagerTypes();

  for (const auto & buildable_type : buildable_types)
  {
    auto & rm_name = std::get<0>(buildable_type);
    auto & rm_type = std::get<1>(buildable_type);
    auto rm_input_parameter_func = std::get<2>(buildable_type);

    addRelationshipManager(moose_object_pars, rm_name, rm_type, rm_input_parameter_func);
  }
}

void
Component::addRelationshipManager(
    const InputParameters & moose_object_pars,
    std::string rm_name,
    Moose::RelationshipManagerType rm_type,
    Moose::RelationshipManagerInputParameterCallback rm_input_parameter_func,
    Moose::RMSystemType)
{
  // These need unique names
  static unsigned int unique_object_id = 0;

  auto new_name = moose_object_pars.get<std::string>("_moose_base") + '_' + name() + '_' + rm_name +
                  "_" + Moose::stringify(rm_type) + " " + std::to_string(unique_object_id);

  auto rm_params = _factory.getValidParams(rm_name);
  rm_params.set<Moose::RelationshipManagerType>("rm_type") = rm_type;

  rm_params.set<std::string>("for_whom") = name();

  // If there is a callback for setting the RM parameters let's use it
  if (rm_input_parameter_func)
    rm_input_parameter_func(moose_object_pars, rm_params);

  rm_params.set<MooseMesh *>("mesh") = &_mesh;

  if (!rm_params.areAllRequiredParamsValid())
    mooseError("Missing required parameters for RelationshipManager " + rm_name + " for object " +
               name());

  auto rm_obj = _factory.create<RelationshipManager>(rm_name, new_name, rm_params);

  const bool added = _app.addRelationshipManager(rm_obj);

  // Delete the resources created on behalf of the RM if it ends up not being added to the App.
  if (!added)
    _factory.releaseSharedObjects(*rm_obj);
  else // we added it
    unique_object_id++;
}

void
Component::checkMutuallyExclusiveParameters(const std::vector<std::string> & params,
                                            bool need_one_specified) const
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

    if (n_provided_params == 0 && need_one_specified)
      logError("One of the parameters ", params_list_string, " must be provided");

    if (n_provided_params != 0)
      logError("Only one of the parameters ", params_list_string, " can be provided");
  }
}
