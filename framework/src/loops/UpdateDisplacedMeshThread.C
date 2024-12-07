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

#include "libmesh/mesh_base.h"
#include "libmesh/numeric_vector.h"
#include "libmesh/transient_system.h"
#include "libmesh/explicit_system.h"

UpdateDisplacedMeshThread::UpdateDisplacedMeshThread(FEProblemBase & fe_problem,
                                                     DisplacedProblem & displaced_problem)
  : ThreadedNodeLoop<NodeRange, NodeRange::const_iterator>(fe_problem),
    _displaced_problem(displaced_problem),
    _ref_mesh(_displaced_problem.refMesh()),
    _nl_soln(_displaced_problem._nl_solution),
    _aux_soln(*_displaced_problem._aux_solution)
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
    _sys_to_nonghost_and_ghost_soln(x._sys_to_nonghost_and_ghost_soln),
    _sys_to_var_num_and_direction(x._sys_to_var_num_and_direction)
{
}

void
UpdateDisplacedMeshThread::init()
{
  std::vector<std::string> & displacement_variables = _displaced_problem._displacements;
  unsigned int num_displacements = displacement_variables.size();
  auto & es = _displaced_problem.es();

  _sys_to_var_num_and_direction.clear();
  _sys_to_nonghost_and_ghost_soln.clear();

  for (unsigned int i = 0; i < num_displacements; i++)
  {
    std::string displacement_name = displacement_variables[i];

    for (const auto sys_num : make_range(es.n_systems()))
    {
      auto & sys = es.get_system(sys_num);
      if (sys.has_variable(displacement_name))
      {
        auto & val = _sys_to_var_num_and_direction[sys.number()];
        val.first.push_back(sys.variable_number(displacement_name));
        val.second.push_back(i);
        break;
      }
    }
  }

  for (const auto & pr : _sys_to_var_num_and_direction)
  {
    auto & sys = es.get_system(pr.first);
    mooseAssert(sys.number() <= _nl_soln.size(),
                "The system number should always be less than or equal to the number of nonlinear "
                "systems. If it is equal, then this system is the auxiliary system");
    const NumericVector<Number> * const nonghost_soln =
        sys.number() < _nl_soln.size() ? _nl_soln[sys.number()] : &_aux_soln;
    _sys_to_nonghost_and_ghost_soln.emplace(
        sys.number(),
        std::make_pair(nonghost_soln,
                       NumericVector<Number>::build(nonghost_soln->comm()).release()));
  }

  ConstNodeRange node_range(_ref_mesh.getMesh().nodes_begin(), _ref_mesh.getMesh().nodes_end());

  for (auto & [sys_num, var_num_and_direction] : _sys_to_var_num_and_direction)
  {
    auto & sys = es.get_system(sys_num);
    AllNodesSendListThread send_list(
        this->_fe_problem, _ref_mesh, var_num_and_direction.first, sys);
    Threads::parallel_reduce(node_range, send_list);
    send_list.unique();
    auto & [soln, ghost_soln] = libmesh_map_find(_sys_to_nonghost_and_ghost_soln, sys_num);
    ghost_soln->init(
        soln->size(), soln->local_size(), send_list.send_list(), true, libMesh::GHOSTED);
    soln->localize(*ghost_soln, send_list.send_list());
  }
}

void
UpdateDisplacedMeshThread::onNode(NodeRange::const_iterator & nd)
{
  Node & displaced_node = *(*nd);

  Node & reference_node = _ref_mesh.nodeRef(displaced_node.id());

  for (auto & [sys_num, var_num_and_direction] : _sys_to_var_num_and_direction)
  {
    auto & var_numbers = var_num_and_direction.first;
    auto & directions = var_num_and_direction.second;
    for (const auto i : index_range(var_numbers))
    {
      const auto direction = directions[i];
      if (reference_node.n_dofs(sys_num, var_numbers[i]) > 0)
        displaced_node(direction) =
            reference_node(direction) +
            (*libmesh_map_find(_sys_to_nonghost_and_ghost_soln, sys_num).second)(
                reference_node.dof_number(sys_num, var_numbers[i], 0));
    }
  }
}

void
UpdateDisplacedMeshThread::join(const UpdateDisplacedMeshThread & /*y*/)
{
}
