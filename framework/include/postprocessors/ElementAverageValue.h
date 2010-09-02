/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Orieneted Simulation Environment */
/*                                                              */
/*            @ 2010 Battelle Energy Alliance, LLC              */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ELEMENTAVERAGEVALUE_H
#define ELEMENTAVERAGEVALUE_H

#include "ElementIntegral.h"

//Forward Declarations
class ElementAverageValue;

template<>
InputParameters validParams<ElementAverageValue>();

/**
 * This postprocessor computes a volume integral of the specified variable.
 *
 * Note that specializations of this integral are possible by deriving from this
 * class and overriding computeQpIntegral().
 */
class ElementAverageValue : public ElementIntegral
{
public:
  ElementAverageValue(std::string name, MooseSystem & moose_system, InputParameters parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();

private:
  Real _volume;
};
 
#endif
