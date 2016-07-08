#include "FluidProperties.h"

template<>
InputParameters validParams<FluidProperties>()
{
  InputParameters params = validParams<GeneralUserObject>();

  params.registerBase("FluidProperties");
  return params;
}

FluidProperties::FluidProperties(const InputParameters & parameters) :
    GeneralUserObject(parameters)
{
}

FluidProperties::~FluidProperties()
{
}

void
FluidProperties::execute()
{
}

void
FluidProperties::initialize()
{
}

void
FluidProperties::finalize()
{
}
