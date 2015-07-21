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


PipeBase::PipeBase(const InputParameters & params) :
    GeometricalComponent(params),
    FlowModel(name(), params)
{
}

PipeBase::~PipeBase()
{
}

void
PipeBase::init()
{
  FlowModel::init();
}

UserObjectName
PipeBase::getFluidPropertiesName() const
{
  return FlowModel::_fp_name;
}
