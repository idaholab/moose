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

#include "EigenvaluePostprocessor.h"

template<>
InputParameters validParams<EigenvaluePostprocessor>()
{
  InputParameters params = validParams<EigenvaluePostprocessor>();
  return params;
}

EigenvaluePostprocessor::EigenvaluePostprocessor(const InputParameters & parameters) :
    EigenvaluePostprocessor(parameters),
    _eigen_values(declareVector("eigen_values"))
{}

void
EigenvaluePostprocessor::initialize()
{}

void
EigenvaluePostprocessor::execute()
{

}
