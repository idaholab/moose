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

//Moose includes
#include "GeneralPostprocessor.h"
#include "MooseSystem.h"

template<>
InputParameters validParams<GeneralPostprocessor>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<Postprocessor>();

  return params;
}

GeneralPostprocessor::GeneralPostprocessor(const std::string & name, MooseSystem &moose_system, InputParameters parameters)
  :Postprocessor(name, moose_system, parameters),
   MooseObject(name, moose_system, parameters),
   PostprocessorInterface(moose_system._postprocessor_data[_tid])
{}

  
