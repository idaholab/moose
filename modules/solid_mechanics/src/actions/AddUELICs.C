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

AddUELICs::AddUELICs(const InputParameters & params) : Action(params) {}

void
AddUELICs::act()
{
  // get all ic blocks
  const auto uel_mesh = std::dynamic_pointer_cast<AbaqusUELMesh>(_mesh);
  if (!uel_mesh)
    mooseError("Must use an AbaqusUELMesh for UEL support.");
  const auto & all_ics = uel_mesh->getICBlocks();

  // create initial conditions
  for (const auto & ic : all_ics)
  {
    const auto type = ic._header.get<std::string>("type");

    if (type == "FIELD")
    {
      const auto var_name = uel_mesh->getVarName(ic._header.get<std::size_t>("variable") - 1);

      auto ic_params = _factory.getValidParams("ConstantIC");
      ic_params.set<VariableName>("variable") = var_name;

      if (ic._data_lines.size() == 0)
        mooseError("Missing data lines for FILED IC on variable ", var_name);
      if (ic._data_lines.size() > 1)
        mooseWarning("Only using the first data line of the FILED IC on variable ", var_name);

      // split line
      std::vector<std::string> col;
      MooseUtils::tokenize(ic._data_lines[0], col, 1, ",");

      ic_params.set<std::vector<BoundaryName>>("boundary") = {col[0]};
      ic_params.set<Real>("value") = MooseUtils::convert<Real>(col[1]);
      _problem->addInitialCondition("ConstantIC", var_name + "_IC", ic_params);
    }
    else
      mooseError("Only type=FIELD ICs are currently supported");
  }
}
