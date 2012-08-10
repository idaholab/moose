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

#ifndef REFERENCEINDICATOR_H
#define REFERENCEINDICATOR_H

#include "ElementIntegralIndicator.h"

class ReferenceIndicator;

template<>
InputParameters validParams<ReferenceIndicator>();

class ReferenceIndicator :
  public ElementIntegralIndicator
{
public:
  ReferenceIndicator(const std::string & name, InputParameters parameters);
  virtual ~ReferenceIndicator(){};

protected:

  virtual Real computeQpIndicator();
};

#endif /* REFERENCEINDICATOR_H */
