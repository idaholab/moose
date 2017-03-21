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

template <>
InputParameters validParams<InversePowerMethod>();

class InversePowerMethod : public EigenExecutionerBase
{
public:
  InversePowerMethod(const InputParameters & parameters);

  virtual void init() override;

  virtual void execute() override;

protected:
  virtual void takeStep();

  /// name of the postprocessor for evaluating |x-xprevious|; empty means that no postprocessor is provided and power iteration will not check convergence based on it
  std::string _solution_diff_name;
  /// postprocessor for evaluating |x-xprevious|
  const PostprocessorValue * _solution_diff;
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
};

#endif // INVERSEPOWERMETHOD_H
