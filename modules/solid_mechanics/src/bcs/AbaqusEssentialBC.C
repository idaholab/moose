//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AbaqusEssentialBC.h"

registerMooseObject("SolidMechanicsApp", AbaqusEssentialBC);

InputParameters
AbaqusEssentialBC::validParams()
{
  InputParameters params = BoundaryCondition::validParams();
  params.addClassDescription(
      "Applies boundary conditions from an Abaqus input read throug AbaqusUELMesh");
  return params;
}

AbaqusEssentialBC::AbaqusEssentialBC(const InputParameters & parameters)
  : BoundaryCondition(parameters, true),
    _uel_mesh(dynamic_cast<AbaqusUELMesh *>(&_mesh)),
    _current_node(_assembly.node())
{
  if (!_uel_mesh)
    mooseError("Must use an AbaqusUELMesh for UEL support.");
}

void
AbaqusEssentialBC::timeStepSetup()
{
  // get field initial conditions
  _bc_data.clear();
  for (const auto & ic : _uel_mesh->getFieldICs())
  {
    const auto var_name = _uel_mesh->getVarName(ic._var);
    auto var = &_sys.getActualFieldVariable<Real>(_tid, var_name);

    for (const auto & [nodeset_name, value] : ic._value)
      for (const auto node_index : ic._nsets.at(nodeset_name))
        _bc_data[node_index].emplace_back(var, value);
  }
}

void
AbaqusEssentialBC::computeValue(NumericVector<Number> & current_solution)
{
  // loop over all variables to preset the solution
  auto it = _ic_data.find(_current_node->id());
  if (it == _ic_data.end())
    return;

  for (auto & [var, value] : it->second)
  {
    if (!var.isNodalDefined())
      return;

    std::cout << "Presetting Node  " << _current_node->id() << " var " << var->name() << " to "
              << value << std::endl;

    const dof_id_type & dof_idx = var.nodalDofIndex();
    current_solution.set(dof_idx, value);
  }
}

Real
AbaqusEssentialBC::computeResidual()
{
  // loop over all variables to preset the solution
  auto it = _ic_data.find(_current_node->id());
  if (it == _ic_data.end())
    return;

  for (auto & [var, value] : it->second)
  {
    if (!var.isNodalDefined())
      return;

    std::cout << "Residual for Node  " << _current_node->id() << " var " << var->name() << " to "
              << value << std::endl;

    const Real res = var->getNodalValue(_current_node) - value;
    setResidual(_sys, res, var);
  }
}
