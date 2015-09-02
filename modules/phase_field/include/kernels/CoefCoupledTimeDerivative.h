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

#ifndef COEFCOUPLEDTIMEDERIVATIVE_H
#define COEFCOUPLEDTIMEDERIVATIVE_H

#include "Kernel.h"

// Forward Declaration
class CoefCoupledTimeDerivative;

template<>
InputParameters validParams<CoefCoupledTimeDerivative>();

/**
 * This calculates the time derivative for a coupled variable
 **/
class CoefCoupledTimeDerivative : public Kernel
{
public:
  CoefCoupledTimeDerivative(const std::string & name, InputParameters parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  Real _coef;
  VariableValue & _v_dot;
  VariableValue & _dv_dot;
  unsigned int _v_var;
};

#endif //COEFCOUPLEDTIMEDERIVATIVE_H
