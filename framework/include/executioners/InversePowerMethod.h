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

#ifndef INVERSEPOWERMETHOD_H
#define INVERSEPOWERMETHOD_H

#include "EigenExecutionerBase.h"

// Forward Declarations
class InversePowerMethod;

template<>
InputParameters validParams<InversePowerMethod>();

class InversePowerMethod : public EigenExecutionerBase
{
public:

  InversePowerMethod(const std::string & name, InputParameters parameters);

  virtual void execute();

protected:
  virtual void takeStep();

  /// postprocessor for evaluating |x-xprevious|
  Real * _solution_diff;
  /// minimum number of power iterations
  const unsigned int & _min_iter;
  /// maximum number of power iterations
  const unsigned int & _max_iter;
  /// convergence tolerance on eigenvalue
  const Real & _eig_check_tol;
  /// convergence tolerance on solution difference
  const Real & _sol_check_tol;
  /// tolerance on each power iteration (always one nonlinear iteration)
  const Real & _pfactor;
  /// indicating if Chebyshev acceleration is turned on
  const bool & _cheb_on;
  /// wheather or not to output solutions during power iteration
  bool _output_pi;
  /// exec flag of the postprocessor of |x-xprevious|
  ExecFlagType _xdiff_execflag;
};

#endif //INVERSEPOWERMETHOD_H
