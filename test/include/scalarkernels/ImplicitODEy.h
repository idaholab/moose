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

#ifndef IMPLICITODEY_H
#define IMPLICITODEY_H


#include "ODEKernel.h"

//Forward Declarations
class ImplicitODEy;

template<>
InputParameters validParams<ImplicitODEy>();

/**
 *
 */
class ImplicitODEy : public ODEKernel
{
public:
  ImplicitODEy(const std::string & name, InputParameters parameters);
  virtual ~ImplicitODEy();

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  unsigned int _x_var;
  VariableValue & _x;
};


#endif /* IMPLICITODEY_H */
