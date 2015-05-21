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

#ifndef DIRK_H
#define DIRK_H

#include "TimeIntegrator.h"

class DIRK;

template<>
InputParameters validParams<DIRK>();

/**
 * Second order diagonally implicit Runge Kutta method (DIRK) with two stages.
 */
class DIRK : public TimeIntegrator
{
public:
  DIRK(const std::string & name, InputParameters parameters);
  virtual ~DIRK();

  virtual int order() { return 2; }
  virtual void computeTimeDerivatives();
  virtual void solve();
  virtual void postStep(NumericVector<Number> & residual);

protected:
  
  //! Indicates stage or, if _stage==3, the update step.
  unsigned int _stage;

  //! Buffer to store non-time residual from first stage solve.
  NumericVector<Number> & _residual_stage1;
  
  //! Buffer to store non-time residual from second stage solve
  NumericVector<Number> & _residual_stage2;
  
  //! Buffer to store solution at beginning of time step
  NumericVector<Number> & _solution_start;
};


#endif /* DIRK_H */
