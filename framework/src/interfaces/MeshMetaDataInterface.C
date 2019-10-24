//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MeshMetaDataInterface.h"
#include "MooseApp.h"
#include "MeshGenerator.h"

MeshMetaDataInterface::MeshMetaDataInterface(const MooseObject * moose_object)
  : Restartable(moose_object->getMooseApp(), NAME, SYSTEM, 0)
{
}

MeshMetaDataInterface::MeshMetaDataInterface(const MeshGenerator * mesh_gen_object)
  : Restartable(mesh_gen_object->getMooseApp(), mesh_gen_object->name(), SYSTEM, 0)
{
}

MeshMetaDataInterface::MeshMetaDataInterface(MooseApp & moose_app)
  : Restartable(moose_app, NAME, SYSTEM, 0)
{
}
