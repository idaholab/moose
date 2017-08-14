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

#include "AllNodesSendListThread.h"
#include "DisplacedProblem.h"
#include "MooseMesh.h"
#include "SubProblem.h"

AllNodesSendListThread::AllNodesSendListThread(FEProblemBase & fe_problem,
                                               MooseMesh & mesh,
                                               const std::vector<unsigned int> & var_nums,
                                               unsigned int system_number)
  : ThreadedNodeLoop<NodeRange, NodeRange::const_iterator>(fe_problem),
    _ref_mesh(mesh),
    _var_nums(var_nums),
    _system_number(system_number),
    _send_list()
{
}

AllNodesSendListThread::AllNodesSendListThread(AllNodesSendListThread & x,
                                               Threads::split split)
  : ThreadedNodeLoop<NodeRange, NodeRange::const_iterator>(x, split),
    _ref_mesh(x._ref_mesh),
    _var_nums(x._var_nums),
    _system_number(x._system_number),
    _send_list()
{
}

void
AllNodesSendListThread::onNode(NodeRange::const_iterator & nd)
{
  Node & displaced_node = *(*nd);

  Node & reference_node = _ref_mesh.nodeRef(displaced_node.id());

  for (unsigned int i = 0; i < _var_nums.size(); i++)
  {
    if (reference_node.n_dofs(_system_number, _var_nums[i]) > 0)
      this->_send_list.push_back(reference_node.dof_number(_system_number, _var_nums[i], 0));
  }
}

void
AllNodesSendListThread::join(const AllNodesSendListThread & y)
{
  // Joining simply requires I add the dof indices from the other object
  this->_send_list.insert(this->_send_list.end(),
                          y._send_list.begin(),
                          y._send_list.end());
}

void
AllNodesSendListThread::unique()
{
  // Sort the send list.  After this duplicated
  // elements will be adjacent in the vector
  std::sort(this->_send_list.begin(),
            this->_send_list.end());

  // Now use std::unique to remove any duplicate entries?  There actually
  // shouldn't be any, since we're hitting each node exactly once.
  // std::vector<dof_id_type>::iterator new_end =
  //   std::unique (this->_send_list.begin(),
  //                this->_send_list.end());

  // If we *do* need to remove duplicate entries, then afterward we should
  // remove the end of the send_list, using the "swap trick" from Effective
  // STL.
  // std::vector<dof_id_type>
  //  (this->send_list.begin(), new_end).swap (this->send_list);
}

const std::vector<dof_id_type> &
AllNodesSendListThread::send_list() const
{
  return this->_send_list;
}
