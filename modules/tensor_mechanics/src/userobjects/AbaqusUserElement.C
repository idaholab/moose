//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUserElement.h"
#include "SystemBase.h"
#include "UELThread.h"

#define QUOTE(macro) stringifyName(macro)

registerMooseObject("TensorMechanicsApp", AbaqusUserElement);

InputParameters
AbaqusUserElement::validParams()
{
  auto params = GeneralUserObject::validParams();
  params += BlockRestrictable::validParams();
  params += TaggingInterface::validParams();
  params.addClassDescription("Coupling UserObject to use Abaqus UEL plugins in MOOSE");

  // execute during residual and Jacobian evaluation
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_KERNELS);
  exec_enum = {EXEC_PRE_KERNELS, EXEC_TIMESTEP_END};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  // Avoid uninitialized residual objects
  params.suppressParameter<bool>("force_preic");

  // coupled variables
  params.addParam<std::vector<NonlinearVariableName>>("variables", "Nonlinear coupled variables");
  // auxiliary variables (including temperature)
  params.addParam<std::vector<AuxVariableName>>(
      "external_fields",
      "Auxiliary field variables (or 'predifined field variables') passed to the UEL plugin. Some "
      "plugins may assume that the first field is temperature when there are multiple external "
      "fields.");

  // UEL plugin file
  params.addRequiredParam<FileName>("plugin", "UEL plugin file");

  params.addRequiredParam<std::vector<Real>>(
      "constant_properties", "Constant mechanical and thermal material properties (PROPS)");
  params.addRequiredParam<unsigned int>("num_state_vars",
                                        "The number of state variables this UMAT is going to use");

  params.addParam<int>("jtype", 0, "Abaqus element type integer");

  return params;
}

AbaqusUserElement::AbaqusUserElement(const InputParameters & params)
  : GeneralUserObject(params),
    BlockRestrictable(this),
    TaggingInterface(this),
    _plugin(getParam<FileName>("plugin")),
    _library(_plugin + std::string("-") + QUOTE(METHOD) + ".plugin"),
    _uel(_library.getFunction<uel_t>("uel_")),
    _moose_mesh(UserObject::_subproblem.mesh()),
    _mesh(_moose_mesh.getMesh()),
    _dim(_moose_mesh.dimension()),
    _variable_names(getParam<std::vector<NonlinearVariableName>>("variables")),
    _aux_variable_names(getParam<std::vector<AuxVariableName>>("external_fields")),
    _sub_ids(blockRestricted() ? blockIDs() : _moose_mesh.meshSubdomains()),
    _props(getParam<std::vector<Real>>("constant_properties")),
    _nprops(_props.size()),
    _nstatev(getParam<unsigned int>("num_state_vars")),
    _statev_index_current(0),
    _statev_index_old(1),
    _jtype(getParam<int>("jtype"))
{
  // coupled variables must be nonlinear scalar fields
  for (const auto & variable_name : _variable_names)
  {
    const auto * var =
        &UserObject::_subproblem.getVariable(0,
                                             variable_name,
                                             Moose::VarKindType::VAR_NONLINEAR,
                                             Moose::VarFieldType::VAR_FIELD_STANDARD);
    _variables.push_back(var);

    // check block restriction
    if (!var->hasBlocks(blockIDs()))
      paramError("variables", "must be defined on all blocks the UEL is operating on.");
  }

  for (const auto & aux_variable_name : _aux_variable_names)
  {
    MooseVariableFEBase * aux_var =
        &UserObject::_subproblem.getVariable(0,
                                             aux_variable_name,
                                             Moose::VarKindType::VAR_AUXILIARY,
                                             Moose::VarFieldType::VAR_FIELD_STANDARD);
    _aux_variables.push_back(aux_var);
    aux_var->sys().addVariableToZeroOnResidual(aux_variable_name);

    // check block restriction
    if (!aux_var->hasBlocks(blockIDs()))
      paramError("aux_variables", "must be defined on all blocks the UEL is operating on.");
  }
}

void
AbaqusUserElement::initialSetup()
{
  setupElemRange();
}

void
AbaqusUserElement::meshChanged()
{
  setupElemRange();
}

void
AbaqusUserElement::initialize()
{
}

void
AbaqusUserElement::execute()
{
  // swap the current and old state data at the end of a converged timestep
  if (_fe_problem.getCurrentExecuteOnFlag() == EXEC_TIMESTEP_END)
  {
    std::swap(_statev_index_old, _statev_index_current);
    return;
  }

  PARALLEL_TRY
  {
    UELThread ut(_fe_problem, *this);
    Threads::parallel_reduce(*_elem_range, ut);
  }
  PARALLEL_CATCH;
}

void
AbaqusUserElement::setupElemRange()
{
  _elem_range =
      std::make_unique<ConstElemRange>(_mesh.active_local_subdomain_set_elements_begin(_sub_ids),
                                       _mesh.active_local_subdomain_set_elements_end(_sub_ids));

  // prepopulate the statev map outside of a threaded region
  if (_nstatev > 0)
    for (const auto & elem : *_elem_range)
    {
      _statev[0][elem->id()].resize(_nstatev);
      _statev[1][elem->id()].resize(_nstatev);
    }
}
