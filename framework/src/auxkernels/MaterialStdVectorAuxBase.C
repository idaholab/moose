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

// MOOSE includes
#include "MaterialStdVectorAuxBase.h"

template <>
InputParameters
validParams<MaterialStdVectorAuxBase<>>()
{
  InputParameters params = validParams<MaterialAuxBase<>>();
  params.addParam<unsigned int>("index", 0, "The index to consider for this kernel");
  return params;
}
