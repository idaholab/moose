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

#ifndef EQUATIONPROBLEM_H
#define EQUATIONPROBLEM_H

#include "FEProblem.h"
#include "NonlinearSystem.h"
#include "MooseEigenSystem.h"

class EquationProblem;

template<>
InputParameters validParams<EquationProblem>();

/**
 * Specialization of SubProblem for solving nonlinear equations plus auxiliary equations
 *
 */
class EquationProblem : public FEProblem
{
public:
  EquationProblem(const InputParameters & parameters);

  virtual ~EquationProblem();

  virtual bool useNonlinear() const { return _use_nonlinear; }
  virtual void useNonlinear(bool use_nonlinear) { _use_nonlinear = use_nonlinear; }

private:
  bool _use_nonlinear;
  
};

#endif /* EQUATIONPROBLEM_H */
