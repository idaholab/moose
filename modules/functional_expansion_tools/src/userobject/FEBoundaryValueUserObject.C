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

// Module includes
#include "FEBoundaryValueUserObject.h"

template <>
InputParameters
validParams<FEBoundaryValueUserObject>()
{
  InputParameters params = validParams<FEBoundaryBaseUserObject>();

  params.addClassDescription("Generates an FE representation for a boundary value condition using "
                             "a 'FunctionSeries'-type Function");

  return params;
}

FEBoundaryValueUserObject::FEBoundaryValueUserObject(const InputParameters & parameters)
  : FEBoundaryBaseUserObject(parameters)
{
  // Nothing here
}
