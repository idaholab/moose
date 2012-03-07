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

#ifndef IMPLICITODEX_H
#define IMPLICITODEX_H


#include "ODEKernel.h"

//Forward Declarations
class ImplicitODEx;

template<>
InputParameters validParams<ImplicitODEx>();

/**
 *
 */
class ImplicitODEx : public ODEKernel
{
public:
  ImplicitODEx(const std::string & name, InputParameters parameters);
  virtual ~ImplicitODEx();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _y_var;
  VariableValue & _y;
};


#endif /* IMPLICITODEX_H */
