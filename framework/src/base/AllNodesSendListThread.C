//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AllNodesSendListThread.h"
#include "MooseMesh.h"

#include "libmesh/dof_map.h"

AllNodesSendListThread::AllNodesSendListThread(FEProblemBase & fe_problem,
                                               const MooseMesh & mesh,
                                               const std::vector<unsigned int> & var_nums,
                                               const System & system)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(fe_problem),
    _ref_mesh(mesh),
    _var_nums(var_nums),
    _system(system),
    _system_number(_system.number()),
    _first_dof(_system.get_dof_map().first_dof()),
    _end_dof(_system.get_dof_map().end_dof()),
    _send_list()
{
  // We may use the same _var_num multiple times, but it's inefficient
  // to examine it multiple times.
  std::sort(this->_var_nums.begin(), this->_var_nums.end());

  std::vector<unsigned int>::iterator new_end =
      std::unique(this->_var_nums.begin(), this->_var_nums.end());

  std::vector<unsigned int>(this->_var_nums.begin(), new_end).swap(this->_var_nums);
}

AllNodesSendListThread::AllNodesSendListThread(AllNodesSendListThread & x, Threads::split split)
  : ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>(x, split),
    _ref_mesh(x._ref_mesh),
    _var_nums(x._var_nums),
    _system(x._system),
    _system_number(_system.number()),
    _first_dof(_system.get_dof_map().first_dof()),
    _end_dof(_system.get_dof_map().end_dof()),
    _send_list()
{
}

void
AllNodesSendListThread::onNode(ConstNodeRange::const_iterator & nd)
{
  const Node & node = *(*nd);

  for (unsigned int i = 0; i < _var_nums.size(); i++)
  {
    if (node.n_dofs(_system_number, _var_nums[i]) > 0)
    {
      const dof_id_type id = node.dof_number(_system_number, _var_nums[i], 0);
      if (id < _first_dof || id >= _end_dof)
        this->_send_list.push_back(id);
    }
  }
}

void
AllNodesSendListThread::join(const AllNodesSendListThread & y)
{
  // Joining simply requires I add the dof indices from the other object
  this->_send_list.insert(this->_send_list.end(), y._send_list.begin(), y._send_list.end());
}

void
AllNodesSendListThread::unique()
{
  // Sort the send list.  After this duplicated
  // elements will be adjacent in the vector
  std::sort(this->_send_list.begin(), this->_send_list.end());

  // Now use std::unique to remove any duplicate entries.  There
  // actually shouldn't be any, since we're hitting each node exactly
  // once and we pre-uniqued _var_nums.
  // std::vector<dof_id_type>::iterator new_end =
  //   std::unique (this->_send_list.begin(),
  //                this->_send_list.end());

  // If we *do* need to remove duplicate entries, then afterward we should
  // remove the end of the send_list, using the "swap trick" from Effective
  // STL.
  // std::vector<dof_id_type>
  //  (this->_send_list.begin(), new_end).swap (this->_send_list);
}

const std::vector<dof_id_type> &
AllNodesSendListThread::send_list() const
{
  return this->_send_list;
}
