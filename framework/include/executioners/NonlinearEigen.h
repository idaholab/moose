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

#ifndef NONLINEAREIGEN_H
#define NONLINEAREIGEN_H

#include "EigenExecutionerBase.h"

// Forward Declarations
class NonlinearEigen;

template <>
InputParameters validParams<NonlinearEigen>();

class NonlinearEigen : public EigenExecutionerBase
{
public:
  NonlinearEigen(const InputParameters & parameters);

  virtual void init() override;

  virtual void execute() override;

protected:
  virtual void takeStep();

  const unsigned int & _free_iter;
  const Real & _abs_tol;
  const Real & _rel_tol;
  const Real & _pfactor;
  bool _output_pi;
  bool _output_after_pi;
};

#endif // NONLINEAREIGEN_H
