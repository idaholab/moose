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

#ifndef COMPUTENODALUserObjectsTHREAD_H
#define COMPUTENODALUserObjectsTHREAD_H

#include "ParallelUniqueId.h"
#include "UserObjectWarehouse.h"
// libMesh includes
#include "node_range.h"

class Problem;
class SubProblem;

class ComputeNodalUserObjectsThread
{
public:
  ComputeNodalUserObjectsThread(SubProblem & problem, std::vector<UserObjectWarehouse> & user_objects, UserObjectWarehouse::GROUP group);
  // Splitting Constructor
  ComputeNodalUserObjectsThread(ComputeNodalUserObjectsThread & x, Threads::split split);

  virtual ~ComputeNodalUserObjectsThread();

  void operator() (const ConstNodeRange & range);

  void join(const ComputeNodalUserObjectsThread & /*y*/);

protected:
  SubProblem & _sub_problem;
  THREAD_ID _tid;

  std::vector<UserObjectWarehouse> & _user_objects;
  UserObjectWarehouse::GROUP _group;
};

#endif //COMPUTENODALUserObjectsTHREAD_H
