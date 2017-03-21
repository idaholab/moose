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

#include "ThreadedNodeLoop.h"

// Forward declarations
class DisplacedProblem;
class UpdateDisplacedMeshThread;
class MooseMesh;

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class NumericVector;
}

class UpdateDisplacedMeshThread
    : public ThreadedNodeLoop<SemiLocalNodeRange, SemiLocalNodeRange::const_iterator>
{
public:
  UpdateDisplacedMeshThread(FEProblemBase & fe_problem, DisplacedProblem & displaced_problem);

  UpdateDisplacedMeshThread(UpdateDisplacedMeshThread & x, Threads::split split);

  virtual void pre() override;

  virtual void onNode(SemiLocalNodeRange::const_iterator & nd) override;

  void join(const UpdateDisplacedMeshThread & /*y*/);

protected:
  DisplacedProblem & _displaced_problem;
  MooseMesh & _ref_mesh;
  const NumericVector<Number> & _nl_soln;
  const NumericVector<Number> & _aux_soln;

private:
  std::vector<unsigned int> _var_nums;
  std::vector<unsigned int> _var_nums_directions;

  std::vector<unsigned int> _aux_var_nums;
  std::vector<unsigned int> _aux_var_nums_directions;

  unsigned int _num_var_nums;
  unsigned int _num_aux_var_nums;

  unsigned int _nonlinear_system_number;
  unsigned int _aux_system_number;
};

#endif /* UPDATEDISPLACEDMESHTHREAD_H */
