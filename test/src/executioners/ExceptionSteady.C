#include "ExceptionSteady.h"

template<>
InputParameters validParams<ExceptionSteady>()
{
  return validParams<Steady>();
}

ExceptionSteady::ExceptionSteady(const std::string & name, InputParameters parameters) :
    Steady(name, parameters)
{
}

ExceptionSteady::~ExceptionSteady()
{
}

void
ExceptionSteady::execute()
{
  try
  {
    Steady::execute();
  }
  catch (MooseException & e)
  {
    Moose::err << "Caught exception " << e << std::endl;
  }
}
