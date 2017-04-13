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

#include "GeneralVectorPostprocessor.h"

template <>
InputParameters
validParams<GeneralVectorPostprocessor>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params += validParams<VectorPostprocessor>();
  return params;
}

GeneralVectorPostprocessor::GeneralVectorPostprocessor(const InputParameters & parameters)
  : GeneralUserObject(parameters), VectorPostprocessor(parameters)
{
}
