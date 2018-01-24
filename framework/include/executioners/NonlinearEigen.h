//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
