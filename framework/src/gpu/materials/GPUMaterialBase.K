//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUMaterialBase.h"

#include "FEProblemBase.h"

MaterialBase::MaterialBase(const MaterialBase & object)
  : MooseObject(object.parameters()),
    BlockRestrictable(object),
    BoundaryRestrictable(object),
    SetupInterface(&object),
    MooseVariableDependencyInterface(&object),
    ScalarCoupleable(&object),
    FunctionInterface(&object),
    DistributionInterface(&object),
    UserObjectInterface(&object),
    TransientInterface(&object),
    PostprocessorInterface(&object),
    VectorPostprocessorInterface(&object),
    DependencyResolverInterface(),
    Restartable(&object, "MaterialBases"),
    MeshChangedInterface(object.parameters()),
    OutputInterface(object.parameters()),
    RandomInterface(object.parameters(), object._fe_problem, object._tid, false),
    ElementIDInterface(&object),
    GeometricSearchInterface(&object),
    ADFunctorInterface(&object),
    SolutionInvalidInterface(&object),
    _subproblem(object._subproblem),
    _fe_problem(object._fe_problem),
    _tid(object._tid),
    _assembly(object._assembly),
    _coord(object._coord),
    _normals(object._normals),
    _mesh(object._mesh),
    _coord_sys(object._coord_sys),
    _compute(object._compute),
    _has_stateful_property(object._has_stateful_property),
    _declare_suffix(object._declare_suffix),
    _force_stateful_init(object._force_stateful_init)
{
}

namespace Moose
{
namespace Kokkos
{

InputParameters
MaterialBase::validParams()
{
  InputParameters params = ::MaterialBase::validParams();

  params.suppressParameter<bool>("use_displaced_mesh");
  params.suppressParameter<bool>("compute");
  params.suppressParameter<std::vector<OutputName>>("outputs");
  params.suppressParameter<std::vector<std::string>>("output_properties");

  params.addPrivateParam<bool>("_kokkos_object", true);
  params.addPrivateParam<bool>("_kokkos_material", true);

  return params;
}

MaterialBase::MaterialBase(const InputParameters & parameters)
  : ::MaterialBase(parameters),
    MeshHolder(*_fe_problem.mesh().getKokkosMesh()),
    AssemblyHolder(_fe_problem.kokkosAssembly()),
    SystemHolder(_fe_problem.getKokkosSystems()),
    _t(TransientInterface::_t),
    _t_old(TransientInterface::_t_old),
    _t_step(TransientInterface::_t_step),
    _dt(TransientInterface::_dt),
    _dt_old(TransientInterface::_dt_old)
{
}

MaterialBase::MaterialBase(const MaterialBase & object)
  : ::MaterialBase(object),
    MeshHolder(object),
    AssemblyHolder(object),
    SystemHolder(object),
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
  if (materialDataType() == Moose::BLOCK_MATERIAL_DATA)
  {
    std::set<dof_id_type> element_ids;

    for (auto block : blockIDs())
      element_ids.insert(_mesh.getKokkosMesh()->getSubdomainElementIDs(block).begin(),
                         _mesh.getKokkosMesh()->getSubdomainElementIDs(block).end());

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

    std::set<Pair<dof_id_type, unsigned int>> element_side_ids;

    for (auto boundary : boundaries)
      for (auto elem_id : _mesh.getBoundaryActiveSemiLocalElemIds(boundary))
      {
        const auto elem = _mesh.elemPtr(elem_id);

        if (elem->processor_id() == processor_id() &&
            restricted_blocks.find(elem->subdomain_id()) != restricted_blocks.end())
          element_side_ids.insert({_mesh.getKokkosMesh()->getElementID(elem),
                                   _mesh.sideWithBoundaryID(elem, boundary)});
      }

    _element_side_ids = element_side_ids;
  }
}

void
MaterialBase::setVariableDependency()
{
  auto needed_moose_vars = getMooseVariableDependencies();

  for (auto & system : kokkosSystems())
    system.setActiveVariables(needed_moose_vars);

  kokkosSystems().copy();
}

void
MaterialBase::setCacheFlags()
{
  for (auto & system : kokkosSystems())
    system.setCacheFlags(blockIDs());
}

} // namespace Kokkos
} // namespace Moose
