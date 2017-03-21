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

UpdateDisplacedMeshThread::UpdateDisplacedMeshThread(FEProblemBase & fe_problem,
                                                     DisplacedProblem & displaced_problem)
  : ThreadedNodeLoop<SemiLocalNodeRange, SemiLocalNodeRange::const_iterator>(fe_problem),
    _displaced_problem(displaced_problem),
    _ref_mesh(_displaced_problem.refMesh()),
    _nl_soln(*_displaced_problem._nl_solution),
    _aux_soln(*_displaced_problem._aux_solution),
    _var_nums(0),
    _var_nums_directions(0),
    _aux_var_nums(0),
    _aux_var_nums_directions(0),
    _num_var_nums(0),
    _num_aux_var_nums(0),
    _nonlinear_system_number(0),
    _aux_system_number(0)
{
}

UpdateDisplacedMeshThread::UpdateDisplacedMeshThread(UpdateDisplacedMeshThread & x,
                                                     Threads::split split)
  : ThreadedNodeLoop<SemiLocalNodeRange, SemiLocalNodeRange::const_iterator>(x, split),
    _displaced_problem(x._displaced_problem),
    _ref_mesh(x._ref_mesh),
    _nl_soln(x._nl_soln),
    _aux_soln(x._aux_soln),
    _var_nums(0),
    _var_nums_directions(0),
    _aux_var_nums(0),
    _aux_var_nums_directions(0),
    _num_var_nums(0),
    _num_aux_var_nums(0),
    _nonlinear_system_number(0),
    _aux_system_number(0)
{
}

void
UpdateDisplacedMeshThread::pre()
{
  std::vector<std::string> & displacement_variables = _displaced_problem._displacements;
  unsigned int num_displacements = displacement_variables.size();

  _var_nums.clear();
  _var_nums_directions.clear();

  _aux_var_nums.clear();
  _aux_var_nums_directions.clear();

  for (unsigned int i = 0; i < num_displacements; i++)
  {
    std::string displacement_name = displacement_variables[i];

    if (_displaced_problem._displaced_nl.sys().has_variable(displacement_name))
    {
      _var_nums.push_back(
          _displaced_problem._displaced_nl.sys().variable_number(displacement_name));
      _var_nums_directions.push_back(i);
    }
    else if (_displaced_problem._displaced_aux.sys().has_variable(displacement_name))
    {
      _aux_var_nums.push_back(
          _displaced_problem._displaced_aux.sys().variable_number(displacement_name));
      _aux_var_nums_directions.push_back(i);
    }
    else
      mooseError("Undefined variable '", displacement_name, "' used for displacements!");
  }

  _num_var_nums = _var_nums.size();
  _num_aux_var_nums = _aux_var_nums.size();

  _nonlinear_system_number = _displaced_problem._displaced_nl.sys().number();
  _aux_system_number = _displaced_problem._displaced_aux.sys().number();
}

void
UpdateDisplacedMeshThread::onNode(SemiLocalNodeRange::const_iterator & nd)
{
  Node & displaced_node = *(*nd);

  Node & reference_node = _ref_mesh.nodeRef(displaced_node.id());

  for (unsigned int i = 0; i < _num_var_nums; i++)
  {
    unsigned int direction = _var_nums_directions[i];
    if (reference_node.n_dofs(_nonlinear_system_number, _var_nums[i]) > 0)
      displaced_node(direction) =
          reference_node(direction) +
          _nl_soln(reference_node.dof_number(_nonlinear_system_number, _var_nums[i], 0));
  }

  for (unsigned int i = 0; i < _num_aux_var_nums; i++)
  {
    unsigned int direction = _aux_var_nums_directions[i];
    if (reference_node.n_dofs(_aux_system_number, _aux_var_nums[i]) > 0)
      displaced_node(direction) =
          reference_node(direction) +
          _aux_soln(reference_node.dof_number(_aux_system_number, _aux_var_nums[i], 0));
  }
}

void
UpdateDisplacedMeshThread::join(const UpdateDisplacedMeshThread & /*y*/)
{
}
