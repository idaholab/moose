//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RedistributeProperties.h"

registerMooseObject("MooseApp", RedistributeProperties);

InputParameters
RedistributeProperties::validParams()
{
  InputParameters params = RelationshipManager::validParams();

  return params;
}

RedistributeProperties::RedistributeProperties(const InputParameters & parameters)
  : RelationshipManager(parameters)
{
}

void
RedistributeProperties::operator()(const MeshBase::const_element_iterator &,
                                   const MeshBase::const_element_iterator &,
                                   processor_id_type,
                                   map_type &)
{
}

std::unique_ptr<GhostingFunctor>
RedistributeProperties::clone() const
{
  return std::make_unique<RedistributeProperties>(*this);
}

std::string
RedistributeProperties::getInfo() const
{
  return "RedistributeProperties";
}

// the LHS ("this" object) in MooseApp::addRelationshipManager is the existing RelationshipManager
// object to which we are comparing the rhs to determine whether it should get added
bool
RedistributeProperties::operator>=(const RelationshipManager & rhs) const
{
  const auto * rm = dynamic_cast<const RedistributeProperties *>(&rhs);

  // All RedistributeProperties objects are effectively equivalent
  return rm;
}

void
RedistributeProperties::addMaterialPropertyStorage(MaterialPropertyStorage & mat_props)
{
  _mat_prop_storages.push_back(&mat_props);
}

void
RedistributeProperties::redistribute()
{
  for (auto mat_props : _mat_prop_storages)
    if (mat_props->hasStatefulProperties())
    {
      libMesh::out << "redistributing mat_props at " << mat_props << std::endl;
    }
}
