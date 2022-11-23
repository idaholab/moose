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

// Forward declarations
class DisplacedProblem;
class UpdateDisplacedMeshThread;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

class UpdateDisplacedMeshThread : public ThreadedNodeLoop<NodeRange, NodeRange::const_iterator>
{
public:
  UpdateDisplacedMeshThread(FEProblemBase & fe_problem, DisplacedProblem & displaced_problem);

  UpdateDisplacedMeshThread(UpdateDisplacedMeshThread & x, Threads::split split);

  virtual void onNode(NodeRange::const_iterator & nd) override;

  void join(const UpdateDisplacedMeshThread & /*y*/);

protected:
  void init();

  DisplacedProblem & _displaced_problem;
  MooseMesh & _ref_mesh;
  const std::vector<const NumericVector<Number> *> & _nl_soln;
  const NumericVector<Number> & _aux_soln;

  // Solution vectors with expanded ghosting, for ReplicatedMesh or
  // for DistributedMesh cases where the standard algebraic ghosting
  // doesn't reach as far as the geometric ghosting
  std::map<unsigned int,
           std::pair<const NumericVector<Number> *, std::shared_ptr<NumericVector<Number>>>>
      _sys_to_nonghost_and_ghost_soln;

private:
  std::map<unsigned int, std::pair<std::vector<unsigned int>, std::vector<unsigned int>>>
      _sys_to_var_num_and_direction;
};
