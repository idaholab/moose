//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshMetaDataAux.h"

registerMooseObject("MooseTestApp", MeshMetaDataAux);

InputParameters
MeshMetaDataAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addParam<std::string>("mesh_generator_name", "Name of mesh meta data");
  params.addParam<std::string>("mesh_meta_data", "Name of mesh meta data");
  return params;
}

MeshMetaDataAux::MeshMetaDataAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    MeshMetaDataInterface(this),
    _data(getMeshProperty<MetaDataGenerator::MeshData>(
        getParam<std::string>("mesh_meta_data"), getParam<std::string>("mesh_generator_name")))
{
}

Real
MeshMetaDataAux::computeValue()
{
  return _data.getMap(_current_elem);
}
