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

#ifndef UPDATEDISPLACEDMESHTHREAD_H
#define UPDATEDISPLACEDMESHTHREAD_H

// MOOSE includes
#include "MooseMesh.h"
#include "ThreadedNodeLoop.h"

// Forward declarations
class DisplacedProblem;
class AllNodesSendListThread;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

class AllNodesSendListThread
    : public ThreadedNodeLoop<NodeRange, NodeRange::const_iterator>
{
public:
  AllNodesSendListThread(FEProblemBase & fe_problem, MooseMesh & mesh,
                         const std::vector<unsigned int> & var_nums,
                         unsigned int system_number);

  AllNodesSendListThread(AllNodesSendListThread & x, Threads::split split);

  virtual void onNode(NodeRange::const_iterator & nd) override;

  void join(const AllNodesSendListThread & y);

  // Make the list sorted and ensure each entry is unique
  void unique();

  // Return the send_list
  const std::vector<dof_id_type> & send_list() const;

protected:
  MooseMesh & _ref_mesh;

private:
  const std::vector<unsigned int> & _var_nums;

  const unsigned int _system_number;

  std::vector<dof_id_type> _send_list;
};

#endif /* UPDATEDISPLACEDMESHTHREAD_H */
