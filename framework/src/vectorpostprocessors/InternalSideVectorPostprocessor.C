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

#include "InternalSideVectorPostprocessor.h"

template<>
InputParameters validParams<InternalSideVectorPostprocessor>()
{
  InputParameters params = validParams<InternalSideUserObject>();
  params += validParams<VectorPostprocessor>();
  return params;
}

InternalSideVectorPostprocessor::InternalSideVectorPostprocessor(const InputParameters & parameters) :
    InternalSideUserObject(parameters),
    VectorPostprocessor(parameters)
{
}


// DEPRECATED CONSTRUCTOR
InternalSideVectorPostprocessor::InternalSideVectorPostprocessor(const std::string & deprecated_name, InputParameters parameters) :
    InternalSideUserObject(deprecated_name, parameters),
    VectorPostprocessor(parameters)
{
}
