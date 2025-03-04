//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddUELICs.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "AbaqusUELMesh.h"

registerMooseAction("SolidMechanicsApp", AddUELICs, "add_ic");

InputParameters
AddUELICs::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Add initial conditions from an Abaqus input");
  return params;
}

AddUELICs::AddUELICs(const InputParameters & params) : Action(params), _uel_mesh(nullptr) {}

void
AddUELICs::setupBoundary(std::size_t var_id,
                         const std::string & boundary_name,
                         const std::vector<Abaqus::Index> & nodeset)
{
  auto & mesh = _mesh->getMesh();
  const auto boundary_id = _mesh->getBoundaryIDs({boundary_name}, true)[0];
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  for (const auto node : nodeset)
  {
    const auto node_elem = mesh.elem_ptr(node);
    // make sure we only add nodes that carry the variable
    std::cout << node << " " << node_elem->subdomain_id() << '\n';
    if ((node_elem->subdomain_id() & (1 << var_id)) == 0)
      mooseError("Trying to apply a boundary condition on a node on which the chosen variable does "
                 "not exist");

    boundary_info.add_node(node_elem->node_ptr(0), boundary_id);
  }

  _uel_mesh->addNodeset(boundary_id);
}

void
AddUELICs::act()
{
  _uel_mesh = dynamic_cast<AbaqusUELMesh *>(_mesh.get());
  if (!_uel_mesh)
    mooseError("Must use an AbaqusUELMesh for UEL support.");

  // create field initial conditions
  for (const auto & ic : _uel_mesh->getFieldICs())
  {
    const auto var_name = _uel_mesh->getVarName(ic._var);

    for (const auto & [nodeset_name, value] : ic._value)
    {
      const auto boundary_name = "abaqus_" + nodeset_name + "_" + var_name;

      auto ic_params = _factory.getValidParams("ConstantIC");
      ic_params.set<VariableName>("variable") = var_name;

      setupBoundary(ic._var, boundary_name, ic._nsets.at(nodeset_name));

      ic_params.set<std::vector<BoundaryName>>("boundary") = {boundary_name};
      ic_params.set<Real>("value") = value;
      _problem->addInitialCondition("ConstantIC", boundary_name + "_IC", ic_params);
    }
  }
}
