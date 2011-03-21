#include "GeneralPostprocessor.h"

template<>
InputParameters validParams<GeneralPostprocessor>()
{
  InputParameters params = validParams<MooseObject>();
  params += validParams<Postprocessor>();

  return params;
}

GeneralPostprocessor::GeneralPostprocessor(const std::string & name, InputParameters parameters) :
    Postprocessor(name, parameters),
    PostprocessorInterface(parameters)
{}
