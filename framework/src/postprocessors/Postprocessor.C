#include "Postprocessor.h"
#include "Problem.h"

// libMesh includes

template<>
InputParameters validParams<Postprocessor>()
{
  InputParameters params = validParams<Object>();
  return params;
}

Postprocessor::Postprocessor(const std::string & name, InputParameters parameters) :
    Object(name, parameters),
    _problem(*parameters.get<Moose::Problem *>("_problem")),
    _tid(parameters.get<THREAD_ID>("_tid"))
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
