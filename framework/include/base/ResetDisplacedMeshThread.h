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

#ifndef RESETDISPLACEDMESHTHREAD_H
#define RESETDISPLACEDMESHTHREAD_H

#include "libmesh/numeric_vector.h"

#include "ThreadedNodeLoop.h"
#include "MooseMesh.h"

class DisplacedProblem;

class ResetDisplacedMeshThread : public ThreadedNodeLoop<NodeRange, NodeRange::const_iterator>
{
public:
  ResetDisplacedMeshThread(FEProblemBase & fe_problem, DisplacedProblem & displaced_problem);

  ResetDisplacedMeshThread(ResetDisplacedMeshThread & x, Threads::split split);

  void onNode(NodeRange::const_iterator & nd);

  void join(const ResetDisplacedMeshThread & /*y*/);

protected:
  DisplacedProblem & _displaced_problem;
  MooseMesh & _ref_mesh;
};

#endif /* RESETDISPLACEDMESHTHREAD_H */
