/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "UpdateDisplacedMeshThread.h"
#include "DisplacedProblem.h"
#include "MooseMesh.h"

#include "SubProblem.h"

UpdateDisplacedMeshThread::UpdateDisplacedMeshThread(DisplacedProblem & problem) :
      _problem(problem),
      _ref_mesh(_problem.refMesh()),
      _nl_soln(*_problem._nl_solution),
      _aux_soln(*_problem._aux_solution)
{
}

void
UpdateDisplacedMeshThread::operator() (const SemiLocalNodeRange & range) const
{
  ParallelUniqueId puid;

  std::vector<std::string> & displacement_variables = _problem._displacements;
  unsigned int num_displacements = displacement_variables.size();

  std::vector<unsigned int> var_nums;
  std::vector<unsigned int> var_nums_directions;

  std::vector<unsigned int> aux_var_nums;
  std::vector<unsigned int> aux_var_nums_directions;

  for (unsigned int i=0; i<num_displacements; i++)
  {
    std::string displacement_name = displacement_variables[i];

    if (_problem._displaced_nl.sys().has_variable(displacement_name))
    {
      var_nums.push_back(_problem._displaced_nl.sys().variable_number(displacement_name));
      var_nums_directions.push_back(i);
    }
    else if (_problem._displaced_aux.sys().has_variable(displacement_name))
    {
      aux_var_nums.push_back(_problem._displaced_aux.sys().variable_number(displacement_name));
      aux_var_nums_directions.push_back(i);
    }
    else
      mooseError("Undefined variable '"<<displacement_name<<"' used for displacements!");
  }

  unsigned int num_var_nums = var_nums.size();
  unsigned int num_aux_var_nums = aux_var_nums.size();

  unsigned int nonlinear_system_number = _problem._displaced_nl.sys().number();
  unsigned int aux_system_number = _problem._displaced_aux.sys().number();

  SemiLocalNodeRange::const_iterator nd = range.begin();

  for (nd = range.begin() ; nd != range.end(); ++nd)
  {
    Node & displaced_node = *(*nd);

    Node & reference_node = _ref_mesh.node(displaced_node.id());

    for (unsigned int i=0; i<num_var_nums; i++)
    {
      unsigned int direction = var_nums_directions[i];
      if (reference_node.n_dofs(nonlinear_system_number, var_nums[i]) > 0)
        displaced_node(direction) = reference_node(direction) + _nl_soln(reference_node.dof_number(nonlinear_system_number, var_nums[i], 0));
    }

    for (unsigned int i=0; i<num_aux_var_nums; i++)
    {
      unsigned int direction = aux_var_nums_directions[i];
      if (reference_node.n_dofs(aux_system_number, aux_var_nums[i]) > 0)
        displaced_node(direction) = reference_node(direction) + _aux_soln(reference_node.dof_number(aux_system_number, aux_var_nums[i], 0));
    }
  }
}

void
UpdateDisplacedMeshThread::operator() (const NodeRange & range) const
{
  NodeRange::const_iterator nd = range.begin();

  for (; nd != range.end(); ++nd)
  {
    Node & displaced_node = **nd;

    // Get the same node from the reference mesh.
    Node & reference_node = _ref_mesh.node(displaced_node.id());

    // Undisplace the node
    for (unsigned int i=0; i<LIBMESH_DIM; ++i)
      displaced_node(i) = reference_node(i);
  }
}
