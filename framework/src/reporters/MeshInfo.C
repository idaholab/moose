//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshInfo.h"
#include "SubProblem.h"
#include "Transient.h"
// #include "NonlinearSystem.h"
// #include "AuxiliarySystem.h"
// #include "EquationSystems.h"

registerMooseObject("MooseApp", MeshInfo);

InputParameters
MeshInfo::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Report the time and iteration information for the simulation.");

  MultiMooseEnum items("num_dofs num_dofs_nonlinear num_dofs_auxiliary num_elements num_nodes");
  params.addParam<MultiMooseEnum>(
      "items",
      items,
      "The iteration information to output, if nothing is provided everything will be output.");
  return params;
}

MeshInfo::MeshInfo(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _items(getParam<MultiMooseEnum>("items")),
    _num_dofs(declareHelper("num_dofs")),
    _num_dofs_nl(declareHelper("num_dofs_nonlinear")),
    _num_dofs_aux(declareHelper("num_dofs_auxiliary")),
    _num_elem(declareHelper("num_elements")),
    _num_node(declareHelper("num_nodes")) /*,
     _equation_systems(_fe_problem.es()),
     _nonlinear_system(_fe_problem.getNonlinearSystem()),
     _aux_system(_fe_problem.getAuxiliarySystem())*/
{
}

void
MeshInfo::execute()
{
  //_num_dofs = _nonlinear_system.n_dofs();
  //_num_elem;
  //_num_node;
}

unsigned int &
MeshInfo::declareHelper(const std::string & item_name)
{
  return (!_items.isValid() || _items.contains(item_name))
             ? declareValue<unsigned int>(item_name, REPORTER_MODE_DISTRIBUTED)
             : _dummy_unsigned_int;
}
