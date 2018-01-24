//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UpdateDisplacedMeshThread.h"

#include "AllNodesSendListThread.h"
#include "DisplacedProblem.h"
#include "MooseMesh.h"

#include "libmesh/numeric_vector.h"

UpdateDisplacedMeshThread::UpdateDisplacedMeshThread(FEProblemBase & fe_problem,
                                                     DisplacedProblem & displaced_problem)
  : ThreadedNodeLoop<NodeRange, NodeRange::const_iterator>(fe_problem),
    _displaced_problem(displaced_problem),
    _ref_mesh(_displaced_problem.refMesh()),
    _nl_soln(*_displaced_problem._nl_solution),
    _aux_soln(*_displaced_problem._aux_solution),
    _nl_ghosted_soln(NumericVector<Number>::build(_nl_soln.comm()).release()),
    _aux_ghosted_soln(NumericVector<Number>::build(_aux_soln.comm()).release()),
    _var_nums(0),
    _var_nums_directions(0),
    _aux_var_nums(0),
    _aux_var_nums_directions(0),
    _num_var_nums(0),
    _num_aux_var_nums(0),
    _nonlinear_system_number(_displaced_problem._displaced_nl.sys().number()),
    _aux_system_number(_displaced_problem._displaced_aux.sys().number())
{
  this->init();
}

UpdateDisplacedMeshThread::UpdateDisplacedMeshThread(UpdateDisplacedMeshThread & x,
                                                     Threads::split split)
  : ThreadedNodeLoop<NodeRange, NodeRange::const_iterator>(x, split),
    _displaced_problem(x._displaced_problem),
    _ref_mesh(x._ref_mesh),
    _nl_soln(x._nl_soln),
    _aux_soln(x._aux_soln),
    _nl_ghosted_soln(x._nl_ghosted_soln),
    _aux_ghosted_soln(x._aux_ghosted_soln),
    _var_nums(x._var_nums),
    _var_nums_directions(x._var_nums_directions),
    _aux_var_nums(x._aux_var_nums),
    _aux_var_nums_directions(x._aux_var_nums_directions),
    _num_var_nums(x._num_var_nums),
    _num_aux_var_nums(x._num_aux_var_nums),
    _nonlinear_system_number(x._nonlinear_system_number),
    _aux_system_number(x._aux_system_number)
{
}

void
UpdateDisplacedMeshThread::init()
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

  ConstNodeRange node_range(_ref_mesh.getMesh().nodes_begin(), _ref_mesh.getMesh().nodes_end());

  {
    AllNodesSendListThread nl_send_list(
        this->_fe_problem, _ref_mesh, _var_nums, _displaced_problem._displaced_nl.sys());
    Threads::parallel_reduce(node_range, nl_send_list);
    nl_send_list.unique();
    _nl_ghosted_soln->init(
        _nl_soln.size(), _nl_soln.local_size(), nl_send_list.send_list(), GHOSTED);
    _nl_soln.localize(*_nl_ghosted_soln, nl_send_list.send_list());
  }

  {
    AllNodesSendListThread aux_send_list(
        this->_fe_problem, _ref_mesh, _aux_var_nums, _displaced_problem._displaced_aux.sys());
    Threads::parallel_reduce(node_range, aux_send_list);
    aux_send_list.unique();
    _aux_ghosted_soln->init(
        _aux_soln.size(), _aux_soln.local_size(), aux_send_list.send_list(), GHOSTED);
    _aux_soln.localize(*_aux_ghosted_soln, aux_send_list.send_list());
  }
}

void
UpdateDisplacedMeshThread::onNode(NodeRange::const_iterator & nd)
{
  Node & displaced_node = *(*nd);

  Node & reference_node = _ref_mesh.nodeRef(displaced_node.id());

  for (unsigned int i = 0; i < _num_var_nums; i++)
  {
    unsigned int direction = _var_nums_directions[i];
    if (reference_node.n_dofs(_nonlinear_system_number, _var_nums[i]) > 0)
      displaced_node(direction) =
          reference_node(direction) +
          (*_nl_ghosted_soln)(reference_node.dof_number(_nonlinear_system_number, _var_nums[i], 0));
  }

  for (unsigned int i = 0; i < _num_aux_var_nums; i++)
  {
    unsigned int direction = _aux_var_nums_directions[i];
    if (reference_node.n_dofs(_aux_system_number, _aux_var_nums[i]) > 0)
      displaced_node(direction) =
          reference_node(direction) +
          (*_aux_ghosted_soln)(reference_node.dof_number(_aux_system_number, _aux_var_nums[i], 0));
  }
}

void
UpdateDisplacedMeshThread::join(const UpdateDisplacedMeshThread & /*y*/)
{
}
