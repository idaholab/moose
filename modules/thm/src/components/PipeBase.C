#include "PipeBase.h"


const std::string PipeBase::_type("pipe");


template<>
InputParameters validParams<PipeBase>()
{
  InputParameters params = validParams<GeometricalComponent>();
  params += validParams<FlowModel>();
  params.addPrivateParam<std::string>("component_type", PipeBase::_type);
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
