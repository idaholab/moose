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

std::string
AddUELICs::setupBoundary(std::size_t var_id,
                         const std::string & var_name,
                         const std::string & nodeset_name)
{
  auto & mesh = _mesh->getMesh();

  const auto boundary_name = "abaqus_" + nodeset_name + "_" + var_name;
  const auto boundary_id = _mesh->getBoundaryIDs({boundary_name}, true)[0];
  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // const auto & nset = std::dynamic_pointer_cast<AbaqusUELMesh>(_mesh)->getNodeSet(nodeset_name);
  // for (const auto node : nset)
  // {
  //   const auto node_elem = mesh.elem_ptr(node);
  //   // make sure we only add nodes that carry the variable
  //   if (node_elem->subdomain_id() & (1 << var_id))
  //     boundary_info.add_node(node_elem->node_ptr(0), boundary_id);
  // }

  _uel_mesh->addNodeset(boundary_id);
  return boundary_name;
}

void
AddUELICs::act()
{
  _uel_mesh = dynamic_cast<AbaqusUELMesh *>(_mesh.get());
  if (!_uel_mesh)
    mooseError("Must use an AbaqusUELMesh for UEL support.");

  // // get all ic blocks
  // const auto & all_ics = _uel_mesh->getICBlocks();

  // // create initial conditions
  // for (const auto & ic : all_ics)
  // {
  //   const auto type = ic._header.get<std::string>("type");

  //   if (type == "FIELD")
  //   {
  //     const auto var_id = ic._header.get<std::size_t>("variable") - 1;
  //     const auto var_name = _uel_mesh->getVarName(var_id);

  //     auto ic_params = _factory.getValidParams("ConstantIC");
  //     ic_params.set<VariableName>("variable") = var_name;

  //     if (ic._data_lines.size() == 0)
  //       mooseError("Missing data lines for FILED IC on variable ", var_name);
  //     if (ic._data_lines.size() > 1)
  //       mooseWarning("Only using the first data line of the FILED IC on variable ", var_name);

  //     // split line
  //     std::vector<std::string> col;
  //     MooseUtils::tokenize(ic._data_lines[0], col, 1, ",");

  //     ic_params.set<std::vector<BoundaryName>>("boundary") = {
  //         setupBoundary(var_id, var_name, col[0])};
  //     ic_params.set<Real>("value") = MooseUtils::convert<Real>(col[1]);
  //     _problem->addInitialCondition("ConstantIC", var_name + "_IC", ic_params);
  //   }
  //   else
  //     mooseError("Only type=FIELD ICs are currently supported");
  // }
}
