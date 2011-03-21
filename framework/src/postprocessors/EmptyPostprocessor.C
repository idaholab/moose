#include "EmptyPostprocessor.h"

// libMesh includes
#include "parallel.h"

template<>
InputParameters validParams<EmptyPostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

EmptyPostprocessor::EmptyPostprocessor(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters)
{}
