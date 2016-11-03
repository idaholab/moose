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

#ifndef EIGENPROBLEM_H
#define EIGENPROBLEM_H

#include "libmesh/libmesh_config.h"

#if LIBMESH_HAVE_SLEPC

#include "FEProblem.h"
#include "NonlinearEigenSystem.h"

class EigenProblem;

template<>
InputParameters validParams<EigenProblem>();

/**
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class EigenProblem : public FEProblem
{
public:
  EigenProblem(const InputParameters & parameters);

  virtual ~EigenProblem();

  virtual void solve() override;
  virtual bool converged() override;
  virtual void outputStep(ExecFlagType type) override;

private:
  NonlinearEigenSystem *_nl_eigen;
};

#endif /* LIBMESH_HAVE_SLEPC */

#endif /* EIGENPROBLEM_H */
