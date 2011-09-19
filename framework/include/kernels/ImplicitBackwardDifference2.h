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

#ifndef IMPLICITBACKWARDDIFFERENCE2_H
#define IMPLICITBACKWARDDIFFERENCE2_H

#include "TimeDerivative.h"

// Forward Declarations
class ImplicitBackwardDifference2;
template<>
InputParameters validParams<ImplicitBackwardDifference2>();

class ImplicitBackwardDifference2 : public TimeDerivative
{
public:
  ImplicitBackwardDifference2(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();

  virtual Real computeQpJacobian();

  bool _start_with_be;
};

#endif //IMPLICITBACKWARDDIFFERENCE2_H
