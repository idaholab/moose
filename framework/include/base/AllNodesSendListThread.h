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

#ifndef ALLNODESSENDLISTTHREAD_H
#define ALLNODESSENDLISTTHREAD_H

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

#endif /* ALLNODESSENDLISTTHREAD_H */
