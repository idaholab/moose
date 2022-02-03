//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseMesh.h"
#include "ThreadedNodeLoop.h"

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

class AllNodesSendListThread
  : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  AllNodesSendListThread(FEProblemBase & fe_problem,
                         const MooseMesh & mesh,
                         const std::vector<unsigned int> & var_nums,
                         const System & system);

  AllNodesSendListThread(AllNodesSendListThread & x, Threads::split split);

  virtual void onNode(ConstNodeRange::const_iterator & nd) override;

  void join(const AllNodesSendListThread & y);

  // Make the list sorted and ensure each entry is unique
  void unique();

  // Return the send_list
  const std::vector<dof_id_type> & send_list() const;

protected:
  const MooseMesh & _ref_mesh;

private:
  std::vector<unsigned int> _var_nums;

  const System & _system;

  const unsigned int _system_number;

  const dof_id_type _first_dof, _end_dof;

  std::vector<dof_id_type> _send_list;
};
