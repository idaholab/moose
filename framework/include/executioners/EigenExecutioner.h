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

#ifndef EIGENEXECUTIONER_H
#define EIGENEXECUTIONER_H

#include "Steady.h"

class InputParameters;
class EigenExecutioner;
class EigenProblem;

template <typename T>
InputParameters validParams();

template <>
InputParameters validParams<EigenExecutioner>();

class EigenExecutioner : public Steady
{
public:
  /**
   * Constructor
   *
   * @param parameters The parameters object holding data for the class to use.
   * @return Whether or not the solve was successful.
   */
  EigenExecutioner(const InputParameters & parameters);

  virtual void execute() override;

protected:
  EigenProblem & _eigen_problem;
};

#endif // EIGENEXECUTIONER_H
