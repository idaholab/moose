//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusUELInitialCondition.h"
#include "AbaqusUELMesh.h"
#include "Moose.h"

#define QUOTE(macro) stringifyName(macro)

registerMooseObject("SolidMechanicsApp", AbaqusUELInitialCondition);

InputParameters
AbaqusUELInitialCondition::validParams()
{
  auto params = NodalUserObject::validParams();
  params.addClassDescription("Add initial conditions from an Abaqus input");

  // later on when we support steps, we need to be able to execute these at timestep begin for new
  // steps
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

AbaqusUELInitialCondition::AbaqusUELInitialCondition(const InputParameters & params)
  : NodalUserObject(params), _uel_mesh(dynamic_cast<AbaqusUELMesh *>(&_mesh))
{
  if (!_uel_mesh)
    mooseError("Must use an AbaqusUELMesh for UEL support.");

  // couple required variables
  for (const auto & ic : _uel_mesh->getFieldICs())
    const auto var_name = _uel_mesh->getVarName(ic._var);
}

void
AbaqusUELInitialCondition::initialize()
{
  // get field initial conditions
  _ic_data.clear();
  for (const auto & ic : _uel_mesh->getFieldICs())
  {
    const auto var_name = _uel_mesh->getVarName(ic._var);
    auto var = &_sys.getActualFieldVariable<Real>(_tid, var_name);

    for (const auto & [nodeset_name, value] : ic._value)
      for (const auto node_index : ic._nsets.at(nodeset_name))
        _ic_data[node_index].emplace_back(var, value);
  }
}

void
AbaqusUELInitialCondition::execute()
{
  auto it = _ic_data.find(_current_node->id());
  if (it == _ic_data.end())
    return;

  for (auto & [var, value] : it->second)
  {
    std::cout << "Node  " << _current_node->id() << " var " << var->name() << " to " << value
              << std::endl;
    var->reinitNode();
    var->computeNodalValues();
    var->setNodalValue(value);

    // We are done, so update the solution vector
    {
      Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
      var->insert(var->sys().solution());
    }
  }
}
