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
#include "libmesh/system.h"
#include "libmesh/equation_systems.h"

registerMooseObject("MooseApp", MeshInfo);

InputParameters
MeshInfo::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Report the time and iteration information for the simulation.");

  MultiMooseEnum items(
      "num_dofs num_dofs_nonlinear num_dofs_auxiliary num_elements num_nodes num_local_dofs "
      "num_local_dofs_nonlinear num_local_dofs_auxiliary num_local_elements num_local_nodes");
  params.addParam<MultiMooseEnum>(
      "items",
      items,
      "The iteration information to output, if nothing is provided everything will be output.");
  return params;
}

MeshInfo::MeshInfo(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _items(getParam<MultiMooseEnum>("items")),
    _num_dofs(declareHelper<unsigned int>("num_dofs", REPORTER_MODE_REPLICATED)),
    _num_dofs_nl(declareHelper<unsigned int>("num_dofs_nonlinear", REPORTER_MODE_REPLICATED)),
    _num_dofs_aux(declareHelper<unsigned int>("num_dofs_auxiliary", REPORTER_MODE_REPLICATED)),
    _num_elem(declareHelper<unsigned int>("num_elements", REPORTER_MODE_REPLICATED)),
    _num_node(declareHelper<unsigned int>("num_nodes", REPORTER_MODE_REPLICATED)),
    _num_local_dofs(declareHelper<unsigned int>("num_local_dofs", REPORTER_MODE_DISTRIBUTED)),
    _num_local_dofs_nl(
        declareHelper<unsigned int>("num_dofs_local_nonlinear", REPORTER_MODE_DISTRIBUTED)),
    _num_local_dofs_aux(
        declareHelper<unsigned int>("num_dofs_local_auxiliary", REPORTER_MODE_DISTRIBUTED)),
    _num_local_elem(declareHelper<unsigned int>("num_local_elements", REPORTER_MODE_DISTRIBUTED)),
    _num_local_node(declareHelper<unsigned int>("num_local_nodes", REPORTER_MODE_DISTRIBUTED)),
    _equation_systems(_fe_problem.es()),
    _nonlinear_system(_fe_problem.es().get_system("nl0")),
    _aux_system(_fe_problem.es().get_system("aux0")),
    _mesh(_fe_problem.mesh().getMesh())
{
}

void
MeshInfo::execute()
{
  _num_dofs_nl = _nonlinear_system.n_dofs();
  _num_dofs_aux = _aux_system.n_dofs();
  _num_dofs = _equation_systems.n_dofs();
  _num_node = _mesh.n_nodes();
  _num_elem = _mesh.n_elem();
  _num_local_dofs_nl = _nonlinear_system.n_local_dofs();
  _num_local_dofs_aux = _aux_system.n_local_dofs();
  _num_local_dofs = _num_local_dofs_nl + _num_local_dofs_aux;
  _num_local_node = _mesh.n_local_nodes();
  _num_local_elem = _mesh.n_local_elem();
}
