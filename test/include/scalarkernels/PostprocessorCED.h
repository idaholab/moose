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
