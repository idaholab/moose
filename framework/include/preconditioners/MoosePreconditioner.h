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

#ifndef MOOSEPRECONDITIONER_H
#define MOOSEPRECONDITIONER_H

#include "MooseObject.h"

class FEProblem;
class MoosePreconditioner;

template<>
InputParameters validParams<MoosePreconditioner>();

/**
 * Base class for MOOSE preconditioners
 */
class MoosePreconditioner : public MooseObject
{
public:
  MoosePreconditioner(const std::string & name, InputParameters params);
  virtual ~MoosePreconditioner();

protected:
  /// Subproblem this preconditioner is part of
  FEProblem & _fe_problem;
};

#endif /* MOOSEPRECONDITIONER_H */
