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

#ifndef ELEMENTH1SEMIERROR_H
#define ELEMENTH1SEMIERROR_H

#include "ElementIntegral.h"

//Forward Declarations
class ElementH1SemiError;

template<>
InputParameters validParams<ElementH1SemiError>();

/**
 * This postprocessor will print out the h1 seminorm between the computed
 * solution and the passed function.
 * ||u,f||h1 is computed as sqrt( (grad u - grad f) * (grad u - grad f) )
 */
class ElementH1SemiError : public ElementIntegral
{
public:
  ElementH1SemiError(const std::string & name, MooseSystem & moose_system, InputParameters parameters);
  
  virtual Real getValue();

protected:
  virtual Real computeQpIntegral();

private:
  Function & _func;
};

#endif //ELEMENTH1SEMIERROR_H
