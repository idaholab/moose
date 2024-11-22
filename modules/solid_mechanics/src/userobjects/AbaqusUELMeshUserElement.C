//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUELMeshUserElement.h"
#include "AbaqusUELMesh.h"
#include "SystemBase.h"
#include "Executioner.h"

#define QUOTE(macro) stringifyName(macro)

registerMooseObject("SolidMechanicsApp", AbaqusUELMeshUserElement);

InputParameters
AbaqusUELMeshUserElement::validParams()
{
  auto params = GeneralUserObject::validParams();
  params.addClassDescription("Coupling UserObject to use Abaqus UEL plugins in MOOSE");

  // execute during residual and Jacobian evaluation
  ExecFlagEnum & exec_enum = params.set<ExecFlagEnum>("execute_on", true);
  exec_enum.addAvailableFlags(EXEC_PRE_KERNELS);
  exec_enum = {EXEC_PRE_KERNELS};
  params.suppressParameter<ExecFlagEnum>("execute_on");

  // Avoid uninitialized residual objects
  params.suppressParameter<bool>("force_preic");

  // UEL type
  params.addRequiredParam<std::string>("uel_type", "UEL type name (from the Abaqus .inp file)");

  // UEL plugin file
  params.addRequiredParam<FileName>("plugin", "UEL plugin file");

  params.addParam<bool>(
      "use_energy", false, "Set to true of the UEL plugin makes use of the ENERGY parameter");

  params.addRelationshipManager("AbaqusUELRelationshipManager", Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC |
                                    Moose::RelationshipManagerType::COUPLING);
  return params;
}

AbaqusUELMeshUserElement::AbaqusUELMeshUserElement(const InputParameters & params)
  : GeneralUserObject(params),
    _uel_type(getParam<std::string>("uel_type")),
    _plugin(getParam<FileName>("plugin")),
    _library(_plugin + std::string("-") + QUOTE(METHOD) + ".plugin"),
    _uel(_library.getFunction<uel_t>("uel_")),
    _uel_mesh(
        [this]()
        {
          auto uel_mesh = dynamic_cast<AbaqusUELMesh *>(&UserObject::_subproblem.mesh());
          if (!uel_mesh)
            mooseError("AbaqusUELMeshUserElement requires an AbaqusUELMesh to be used.");
          return uel_mesh;
        }()),
    _uel_definition(_uel_mesh->getUEL(_uel_type)),
    _use_energy(getParam<bool>("use_energy"))
{
  // get coupled variables from UEL type

  for (const auto & variable_name : _variable_names)
  {
    const auto * var = &UserObject::_subproblem.getVariable(
        0, variable_name, Moose::VarKindType::VAR_SOLVER, Moose::VarFieldType::VAR_FIELD_STANDARD);
    _variables.push_back(var);
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
  }
}

void
AbaqusUELMeshUserElement::timestepSetup()
{
  // swap the current and old state data after a converged timestep
  if (_app.getExecutioner()->lastSolveConverged())
  {
    std::swap(_statev_index_old, _statev_index_current);
    if (_use_energy)
      for (const auto & [key, value] : _energy)
        _energy_old[key] = value;
  }
  else
  {
    // last timestep did not converge, restore energy from energy_old
    if (_use_energy)
      for (const auto & [key, value] : _energy_old)
        _energy[key] = value;
  }
}
void
AbaqusUELMeshUserElement::initialSetup()
{
  setupElemRange();
}

void
AbaqusUELMeshUserElement::meshChanged()
{
  setupElemRange();
}

void
AbaqusUELMeshUserElement::initialize()
{
}

void
AbaqusUELMeshUserElement::execute()
{
  // PARALLEL_TRY
  // {
  //   UELMeshThread ut(_fe_problem, *this);
  //   Threads::parallel_reduce(*_elem_range, ut);

  //   // copy over PNEWDT
  //   _pnewdt = ut._min_pnewdt;
  // }
  // PARALLEL_CATCH;
}

void
AbaqusUELMeshUserElement::setupElemRange()
{

  // prepopulate the statev map outside of a threaded region
  // if (_nstatev > 0)
  //   for (const auto & elem : *_elem_range)
  //   {
  //     _statev[0][elem->id()].resize(_nstatev);
  //     _statev[1][elem->id()].resize(_nstatev);
  //   }
}

const std::array<Real, 8> *
AbaqusUELMeshUserElement::getUELEnergy(dof_id_type element_id) const
{
  const auto it = _energy.find(element_id);

  // if this UO does not have the data for the requested element we return null
  // this allows the querying object to try multiple (block restricted) AbaqusUELMeshUserElement
  // user objects until it finds the value (or else error out)
  if (it == _energy.end())
    return nullptr;

  return &it->second;
}
