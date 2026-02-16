//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosMaterialBase.h"

#include "FEProblemBase.h"

namespace Moose::Kokkos
{

InputParameters
MaterialBase::validParams()
{
  InputParameters params = ::MaterialBase::validParams();

  MooseEnum constant_option("NONE=0 ELEMENT=1 SUBDOMAIN=2", "NONE");
  params.addParam<MooseEnum>(
      "constant_on",
      constant_option,
      "When ELEMENT or SUBDOMAIN, computeQpProperties() will be only called once for each "
      "element/face or subdomain. Material properties will be only stored for each element/face or "
      "subdomain accordingly. The 'qp' argument wlll be ignored.");

  params.suppressParameter<bool>("use_displaced_mesh");
  params.suppressParameter<bool>("compute");
  params.suppressParameter<std::vector<OutputName>>("outputs");
  params.suppressParameter<std::vector<std::string>>("output_properties");

  params.addPrivateParam<bool>(MooseBase::kokkos_object_param, true);
  params.addPrivateParam<bool>("_kokkos_material", true);

  return params;
}

MaterialBase::MaterialBase(const InputParameters & parameters)
  : ::MaterialBase(parameters),
    MeshHolder(*_fe_problem.mesh().getKokkosMesh()),
    AssemblyHolder(_fe_problem.kokkosAssembly()),
    SystemHolder(_fe_problem.getKokkosSystems()),
    _constant_option(getParam<MooseEnum>("constant_on").getEnum<PropertyConstantOption>()),
    _t(TransientInterface::_t),
    _t_old(TransientInterface::_t_old),
    _t_step(TransientInterface::_t_step),
    _dt(TransientInterface::_dt),
    _dt_old(TransientInterface::_dt_old)
{
}

MaterialBase::MaterialBase(const MaterialBase & object)
  : ::MaterialBase(object, {}),
    MeshHolder(object),
    AssemblyHolder(object),
    SystemHolder(object),
    _constant_option(object._constant_option),
    _t(object._t),
    _t_old(object._t_old),
    _t_step(object._t_step),
    _dt(object._dt),
    _dt_old(object._dt_old)
{
  _element_ids = object._element_ids;
  _element_side_ids = object._element_side_ids;
}

void
MaterialBase::initialSetup()
{
  if (_constant_option == PropertyConstantOption::SUBDOMAIN)
  {
    auto & restricted_blocks = boundaryRestricted() ? meshBlockIDs() : blockIDs();

    std::set<ContiguousElementID> element_ids;

    for (auto block : restricted_blocks)
    {
      auto first_elem = *_mesh.getKokkosMesh()->getSubdomainContiguousElementIDRange(block).begin();
      if (first_elem != libMesh::DofObject::invalid_id)
        element_ids.insert(first_elem);
    }

    _element_ids = element_ids;
  }
  else if (materialDataType() == Moose::BLOCK_MATERIAL_DATA)
  {
    std::set<ContiguousElementID> element_ids;

    for (auto block : blockIDs())
      element_ids.insert(_mesh.getKokkosMesh()->getSubdomainContiguousElementIDRange(block).begin(),
                         _mesh.getKokkosMesh()->getSubdomainContiguousElementIDRange(block).end());

    _element_ids = element_ids;
  }
  else
  {
    auto & restricted_blocks = boundaryRestricted() ? meshBlockIDs() : blockIDs();
    auto & restricted_boundaries = boundaryRestricted() ? boundaryIDs() : meshBoundaryIDs();
    auto & material_boundaries = kokkosAssembly().getMaterialBoundaries();

    std::vector<BoundaryID> boundaries;

    std::set_intersection(restricted_boundaries.begin(),
                          restricted_boundaries.end(),
                          material_boundaries.begin(),
                          material_boundaries.end(),
                          std::back_inserter(boundaries));

    std::set<Pair<ContiguousElementID, unsigned int>> element_side_ids;

    for (auto boundary : boundaries)
      for (auto elem_id : _mesh.getBoundaryActiveSemiLocalElemIds(boundary))
      {
        const auto elem = _mesh.elemPtr(elem_id);

        if (elem->processor_id() == processor_id() &&
            restricted_blocks.find(elem->subdomain_id()) != restricted_blocks.end())
          element_side_ids.insert({_mesh.getKokkosMesh()->getContiguousElementID(elem),
                                   _mesh.sideWithBoundaryID(elem, boundary)});
      }

    _element_side_ids = element_side_ids;
  }
}

} // namespace Moose::Kokkos
