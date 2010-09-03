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

#ifndef FUNCTIONNEUMANNBC_H
#define FUNCTIONNEUMANNBC_H

#include "BoundaryCondition.h"

//Forward Declarations
class FunctionNeumannBC;
class Function;

template<>
InputParameters validParams<FunctionNeumannBC>();


class FunctionNeumannBC : public BoundaryCondition
{
public:

  FunctionNeumannBC(std::string name,
             MooseSystem &sys,
             InputParameters parameters);

protected:

  virtual Real computeQpResidual();

private:
  Function & _func;
};
#endif //FUNCTIONNEUMANNBC_H
