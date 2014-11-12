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

#include "libmesh/numeric_vector.h"

#include "MooseMesh.h"

class DisplacedProblem;

class UpdateDisplacedMeshThread
{
public:
  UpdateDisplacedMeshThread(DisplacedProblem & problem);

  void operator() (const SemiLocalNodeRange & range) const;
  void operator() (const NodeRange & range) const;

protected:
  DisplacedProblem & _problem;
  MooseMesh & _ref_mesh;
  const NumericVector<Number> & _nl_soln;
  const NumericVector<Number> & _aux_soln;
};

#endif /* UPDATEDISPLACEDMESHTHREAD_H */
