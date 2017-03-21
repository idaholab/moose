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

#include "ElementH1Error.h"
#include "Function.h"

template <>
InputParameters
validParams<ElementH1Error>()
{
  InputParameters params = validParams<ElementW1pError>();
  return params;
}

ElementH1Error::ElementH1Error(const InputParameters & parameters) : ElementW1pError(parameters) {}
