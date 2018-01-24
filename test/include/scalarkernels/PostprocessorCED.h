//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POSTPROCESSORCED_H
#define POSTPROCESSORCED_H

#include "ScalarKernel.h"

class PostprocessorCED;

template <>
InputParameters validParams<PostprocessorCED>();

/**
 *
 */
class PostprocessorCED : public ScalarKernel
{
public:
  PostprocessorCED(const InputParameters & parameters);
  virtual ~PostprocessorCED();

  virtual void reinit();
  virtual void computeResidual();
  virtual void computeJacobian();
  virtual void computeOffDiagJacobian(unsigned int jvar);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _i;

  Real _value;
  const PostprocessorValue & _pp_value;
};

#endif /* POSTPROCESSORCED_H */
