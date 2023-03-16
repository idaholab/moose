//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructureFromFile3D.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/exodusII_io_helper.h"

registerMooseObject("ThermalHydraulicsApp", HeatStructureFromFile3D);

InputParameters
HeatStructureFromFile3D::validParams()
{
  InputParameters params = FileMeshComponent::validParams();
  params += HeatStructureInterface::validParams();

  params.addClassDescription("Heat structure component that loads a 3D mesh from an ExodusII file");

  return params;
}

HeatStructureFromFile3D::HeatStructureFromFile3D(const InputParameters & params)
  : FileMeshComponent(params), HeatStructureInterface(this)
{
  if (_file_is_readable)
  {
    ExodusII_IO_Helper exio_helper(*this, false, true, false);
    exio_helper.open(_file_name.c_str(), true);
    exio_helper.read_and_store_header_info();
    if (exio_helper.num_dim != 3)
      logError("File '", _file_name, "' does not contain a 3D mesh.");
  }
}

bool
HeatStructureFromFile3D::hasRegion(const std::string & name) const
{
  checkSetupStatus(MESH_PREPARED);

  return std::find(_region_names.begin(), _region_names.end(), name) != _region_names.end();
}

void
HeatStructureFromFile3D::setupMesh()
{
  _region_names = buildMesh();
}

void
HeatStructureFromFile3D::init()
{
  FileMeshComponent::init();
  HeatStructureInterface::init();
}

void
HeatStructureFromFile3D::check() const
{
  FileMeshComponent::check();
  HeatStructureInterface::check();
}

void
HeatStructureFromFile3D::addVariables()
{
  HeatStructureInterface::addVariables();
}

void
HeatStructureFromFile3D::addMooseObjects()
{
  HeatStructureInterface::addMooseObjects();
}
