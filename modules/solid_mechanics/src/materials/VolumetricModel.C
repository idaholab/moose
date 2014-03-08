#include "VolumetricModel.h"

template<>
InputParameters validParams<VolumetricModel>()
{
  return validParams<Material>();
}

VolumetricModel::VolumetricModel( const std::string & name,
                                  InputParameters & parameters ):
  Material( name, parameters )
{}

VolumetricModel::~VolumetricModel() {}
