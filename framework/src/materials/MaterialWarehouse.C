//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MaterialWarehouse.h"
#include "MaterialBase.h"

void
MaterialWarehouse::addObjects(std::shared_ptr<MaterialBase> block,
                              std::shared_ptr<MaterialBase> neighbor,
                              std::shared_ptr<MaterialBase> face,
                              THREAD_ID tid /*=0*/)
{
  MooseObjectWarehouse<MaterialBase>::addObject(block, tid);
  _neighbor_materials.addObject(neighbor, tid);
  _face_materials.addObject(face, tid);
}

void
MaterialWarehouse::addInterfaceObject(std::shared_ptr<MaterialBase> interface, THREAD_ID tid /*=0*/)
{
  _interface_materials.addObject(interface, tid);
}

const MooseObjectWarehouse<MaterialBase> &
MaterialWarehouse::operator[](Moose::MaterialDataType data_type) const
{
  switch (data_type)
  {
    case Moose::NEIGHBOR_MATERIAL_DATA:
      return _neighbor_materials;

    case Moose::FACE_MATERIAL_DATA:
      return _face_materials;

    case Moose::INTERFACE_MATERIAL_DATA:
      return _interface_materials;

    default:
      return *this;
  }
}

void
MaterialWarehouse::initialSetup(THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<MaterialBase>::initialSetup(tid);
  _neighbor_materials.initialSetup(tid);
  _face_materials.initialSetup(tid);
  _interface_materials.initialSetup(tid);
}

void
MaterialWarehouse::timestepSetup(THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<MaterialBase>::timestepSetup(tid);
  _neighbor_materials.timestepSetup(tid);
  _face_materials.timestepSetup(tid);
  _interface_materials.timestepSetup(tid);
}

void
MaterialWarehouse::subdomainSetup(THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<MaterialBase>::subdomainSetup(tid);
  _face_materials.subdomainSetup(tid);
}

void
MaterialWarehouse::neighborSubdomainSetup(THREAD_ID tid /*=0*/) const
{
  _neighbor_materials.subdomainSetup(tid);
}

void
MaterialWarehouse::subdomainSetup(SubdomainID id, THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<MaterialBase>::subdomainSetup(id, tid);
  _face_materials.subdomainSetup(id, tid);
}

void
MaterialWarehouse::neighborSubdomainSetup(SubdomainID id, THREAD_ID tid /*=0*/) const
{
  _neighbor_materials.subdomainSetup(id, tid);
}

void
MaterialWarehouse::residualSetup(THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<MaterialBase>::residualSetup(tid);
  _neighbor_materials.residualSetup(tid);
  _face_materials.residualSetup(tid);
  _interface_materials.residualSetup(tid);
}

void
MaterialWarehouse::jacobianSetup(THREAD_ID tid /*=0*/) const
{
  MooseObjectWarehouse<MaterialBase>::jacobianSetup(tid);
  _neighbor_materials.jacobianSetup(tid);
  _face_materials.jacobianSetup(tid);
  _interface_materials.jacobianSetup(tid);
}

void
MaterialWarehouse::updateActive(THREAD_ID tid /*=0*/)
{
  MooseObjectWarehouse<MaterialBase>::updateActive(tid);
  _neighbor_materials.updateActive(tid);
  _face_materials.updateActive(tid);
  _interface_materials.updateActive(tid);
}

void
MaterialWarehouse::sort(THREAD_ID tid /*=0*/)
{
  checkThreadID(tid);

  for (auto & object_pair : _all_block_objects[tid])
    sortHelper(object_pair.second);
  for (auto & object_pair : _all_boundary_objects[tid])
    sortHelper(object_pair.second);

  for (auto & object_pair : _neighbor_materials._all_block_objects[tid])
    sortHelper(object_pair.second);
  for (auto & object_pair : _neighbor_materials._all_boundary_objects[tid])
    sortHelper(object_pair.second);

  for (auto & object_pair : _face_materials._all_block_objects[tid])
    sortHelper(object_pair.second);
  for (auto & object_pair : _face_materials._all_boundary_objects[tid])
    sortHelper(object_pair.second);

  for (auto & object_pair : _interface_materials._all_block_objects[tid])
    sortHelper(object_pair.second);
  for (auto & object_pair : _interface_materials._all_boundary_objects[tid])
    sortHelper(object_pair.second);

  updateActive(tid);
}
