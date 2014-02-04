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

#ifndef FDADVECTION_H
#define FDADVECTION_H

#include "FDKernel.h"

class FDAdvection;

template<>
InputParameters validParams<FDAdvection>();

class FDAdvection : public FDKernel
{
public:

  FDAdvection(const std::string & name,
             InputParameters parameters);

protected:

  virtual Real computeQpResidual();

private:

  VariableGradient & _grad_advector;
};

#endif //FDADVECTION_H
