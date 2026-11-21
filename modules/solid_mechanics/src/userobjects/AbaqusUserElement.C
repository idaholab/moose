//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUserElement.h"
#include "SystemBase.h"
#include "UELThread.h"

registerMooseObject("SolidMechanicsApp", AbaqusUserElement);

InputParameters
AbaqusUserElement::validParams()
{
  auto params = AbaqusUELUserObjectBase<dof_id_type, std::map>::validParams();
  params += BlockRestrictable::validParams();
  params.addClassDescription("Coupling UserObject to use Abaqus UEL plugins in MOOSE");

  // coupled variables
  params.addParam<std::vector<NonlinearVariableName>>("variables", "Nonlinear coupled variables");

  params.addRequiredParam<std::vector<Real>>(
      "constant_properties", "Constant mechanical and thermal material properties (PROPS)");
  params.addRequiredParam<unsigned int>("num_state_vars",
                                        "The number of state variables this UMAT is going to use");

  params.addParam<int>("jtype", 0, "Abaqus element type integer");

  return params;
}

AbaqusUserElement::AbaqusUserElement(const InputParameters & params)
  : AbaqusUELUserObjectBase<dof_id_type, std::map>(params,
                                                   params.get<unsigned int>("num_state_vars")),
    BlockRestrictable(this),
    _moose_mesh(UserObject::_subproblem.mesh()),
    _mesh(_moose_mesh.getMesh()),
    _dim(_moose_mesh.dimension()),
    _variable_names(getParam<std::vector<NonlinearVariableName>>("variables")),
    _aux_variable_names(getParam<std::vector<AuxVariableName>>("external_fields")),
    _sub_ids(blockRestricted() ? blockIDs() : _moose_mesh.meshSubdomains()),
    _props(getParam<std::vector<Real>>("constant_properties")),
    _nprops(_props.size()),
    _jtype(getParam<int>("jtype"))
{
  // coupled variables must be nonlinear scalar fields
  for (const auto & variable_name : _variable_names)
  {
    const auto * var = &UserObject::_subproblem.getVariable(
        0, variable_name, Moose::VarKindType::VAR_SOLVER, Moose::VarFieldType::VAR_FIELD_STANDARD);
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

    // check block restriction
    if (!aux_var->hasBlocks(blockIDs()))
      paramError("external_fields", "must be defined on all blocks the UEL is operating on.");
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
  PARALLEL_TRY
  {
    UELThread ut(_fe_problem, *this);
    Threads::parallel_reduce(*_elem_range, ut);

    // copy over PNEWDT
    _pnewdt = ut.minPNewDt();
  }
  PARALLEL_CATCH;
}

void
AbaqusUserElement::setupElemRange()
{
  _elem_range =
      std::make_unique<ConstElemRange>(_mesh.active_local_subdomain_set_elements_begin(_sub_ids),
                                       _mesh.active_local_subdomain_set_elements_end(_sub_ids));

  // prepopulate the statev and energy maps outside of a threaded region
  for (const auto & elem : *_elem_range)
  {
    if (_nstatev > 0)
    {
      _statev[0][elem->id()].resize(_nstatev);
      _statev[1][elem->id()].resize(_nstatev);
    }
    if (_use_energy)
    {
      _energy.try_emplace(elem->id());
      _energy_old.try_emplace(elem->id());
    }
  }
}
