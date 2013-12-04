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

#ifndef FUNCTIONCONTROLLEDDT_H
#define FUNCTIONCONTROLLEDDT_H

#include "SolutionTimeAdaptiveDT.h"
#include "Function.h"
#include "FunctionInterface.h"

class FunctionControlledDT;

template<>
InputParameters validParams<FunctionControlledDT>();

class FunctionControlledDT : public SolutionTimeAdaptiveDT, protected FunctionInterface
{
public:
  FunctionControlledDT(const std::string & name, InputParameters parameters);
  virtual ~FunctionControlledDT();

  virtual void init();
  virtual void preSolve();
  virtual void postSolve();
  virtual void rejectStep();

protected:
  virtual Real computeInitialDT();
  virtual Real computeDT();

  Real bounddt(Real tentativedt);

  const std::vector<Real> & _maximums;
  const std::vector<Real> & _minimums;
  Real _decrement;
  Real _increment;
  // True if cut back of the time step occurred
  bool _cutback_occurred;

  std::vector<Function *> _f;
  Real _maxdt;
  Real _mindt;
};

#endif /* FUNCTIONCONTROLLEDDT_H_ */
