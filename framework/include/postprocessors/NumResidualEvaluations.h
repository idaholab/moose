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

#ifndef NUMRESIDUALEVALUATIONS_H
#define NUMRESIDUALEVALUATIONS_H

#include "GeneralPostprocessor.h"

//Forward Declarations
class NumResidualEvaluations;

template<>
InputParameters validParams<NumResidualEvaluations>();

/**
 * Just returns the total number of Residual Evaluations performed.
 */
class NumResidualEvaluations : public GeneralPostprocessor
{
public:
  NumResidualEvaluations(const std::string & name, InputParameters parameters);

  virtual void initialize() {}
  virtual void execute() {}

  /**
   * This will return the final nonlinear residual.
   */
  virtual Real getValue();
};

#endif //NUMRESIDUALEVALUATIONS_H
