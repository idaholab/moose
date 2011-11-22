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

#include "Postprocessor.h"
#include "SubProblem.h"

// libMesh includes

template<>
InputParameters validParams<Postprocessor>()
{
  InputParameters params = validParams<MooseObject>();

  params.addParam<bool>("output", true, "If the postprocessor value is outputted or not (to screen and files)");
  params.addPrivateParam<bool>("use_displaced_mesh", false);
  params.addPrivateParam<std::string>("built_by_action", "add_postprocessor");
  return params;
}

Postprocessor::Postprocessor(const std::string & name, InputParameters parameters) :
    MooseObject(name, parameters),
    _problem(*parameters.get<Problem *>("_problem")),
    _subproblem(*parameters.get<SubProblem *>("_subproblem")),
    _tid(parameters.get<THREAD_ID>("_tid")),
    _output(parameters.get<bool>("output"))

//   _local_name(name),
//   _local_tid(parameters.get<THREAD_ID>("_tid")),
{
  // Initialize the postprocessor data for this PP
  // FIXME: PPS::init()
//  parameters.get<MooseSystem *>("_moose_system")->_postprocessor_data[_local_tid].init(name);
}

//const std::string &
//Postprocessor::name()
//{
//  return _local_name;
//}
