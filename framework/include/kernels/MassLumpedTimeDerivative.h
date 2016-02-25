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

#ifndef MASSLUMPEDTIMEDERIVATIVE_H
#define MASSLUMPEDTIMEDERIVATIVE_H

#include "TimeKernel.h"

// Forward Declaration
class MassLumpedTimeDerivative;

template<>
InputParameters validParams<MassLumpedTimeDerivative>();

class MassLumpedTimeDerivative : public TimeKernel
{
public:
  MassLumpedTimeDerivative(const InputParameters & parameters);

  virtual void computeJacobian();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const VariableValue & _u_dot_nodal;
};

#endif // MASSLUMPEDTIMEDERIVATIVE_H
