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

#ifndef ALPHACED_H
#define ALPHACED_H

#include "ScalarKernel.h"

class AlphaCED;

template<>
InputParameters validParams<AlphaCED>();

/**
 *
 */
class AlphaCED : public ScalarKernel
{
public:
  AlphaCED(const std::string & name, InputParameters parameters);
  virtual ~AlphaCED();

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
};

#endif /* ALPHACED_H */
