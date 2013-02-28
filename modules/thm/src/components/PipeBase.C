#include "PipeBase.h"


const std::string PipeBase::_type("pipe");


template<>
InputParameters validParams<PipeBase>()
{
  InputParameters params = validParams<GeometricalComponent>();
  params.addParam("component_type", PipeBase::_type, "The type of the component");
  //Input parameters [NO] default values should be given.

  return params;
}


PipeBase::PipeBase(const std::string & name, InputParameters params) :
    GeometricalComponent(name, params),
    FlowModel(params)
{
}

PipeBase::~PipeBase()
{
}
