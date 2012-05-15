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

#ifndef SINGLEMATRIXPRECONDITIONER_H
#define SINGLEMATRIXPRECONDITIONER_H

#include "MoosePreconditioner.h"

class SingleMatrixPreconditioner;

template<>
InputParameters validParams<SingleMatrixPreconditioner>();

/**
 * Single matrix preconditioner.
 */
class SingleMatrixPreconditioner : public MoosePreconditioner
{
public:
  SingleMatrixPreconditioner(const std::string & name, InputParameters params);
  virtual ~SingleMatrixPreconditioner();
};

#endif /* SINGLEMATRIXPRECONDITIONER_H_ */
