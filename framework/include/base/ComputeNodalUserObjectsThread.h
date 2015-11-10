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

#include "ThreadedNodeLoop.h"
#include "UserObjectWarehouse.h"

// libMesh includes
#include "libmesh/node_range.h"

// Forward declarations
class SubProblem;

class ComputeNodalUserObjectsThread : public ThreadedNodeLoop<ConstNodeRange, ConstNodeRange::const_iterator>
{
public:
  ComputeNodalUserObjectsThread(FEProblem & fe_problem, std::vector<UserObjectWarehouse> & user_objects, UserObjectWarehouse::GROUP group);
  // Splitting Constructor
  ComputeNodalUserObjectsThread(ComputeNodalUserObjectsThread & x, Threads::split split);

  virtual ~ComputeNodalUserObjectsThread();

  virtual void onNode(ConstNodeRange::const_iterator & node_it);

  void join(const ComputeNodalUserObjectsThread & /*y*/);

protected:
  std::vector<UserObjectWarehouse> & _user_objects;
  UserObjectWarehouse::GROUP _group;
};

#endif //COMPUTENODALUserObjectsTHREAD_H
