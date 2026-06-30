//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearSystem.h"

#include "Attributes.h"
#include "KokkosLinearFVElementalKernel.h"
#include "KokkosLinearFVFluxKernel.h"
#include "KokkosLinearFVBoundaryCondition.h"
#include "MooseUtils.h"
#include "MaterialPropertyInterface.h"
#include "MaterialBase.h"
#include "FEProblemBase.h"

#include "libmesh/equation_systems.h"

void
LinearSystem::addKokkosKernel(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters & parameters)
{
  auto kernel = _factory.create<Moose::Kokkos::LinearFVKernel>(kernel_name, name, parameters);

  if (!std::dynamic_pointer_cast<Moose::Kokkos::LinearFVElementalKernel>(kernel) &&
      !std::dynamic_pointer_cast<Moose::Kokkos::LinearFVFluxKernel>(kernel))
    mooseError("Unknown Kokkos LinearFVKernel type for object named ", kernel->name());

  _fe_problem.theWarehouse().add(kernel);
}

void
LinearSystem::addKokkosBoundaryCondition(const std::string & bc_name,
                                         const std::string & name,
                                         InputParameters & parameters)
{
  auto bc = _factory.create<Moose::Kokkos::LinearFVBoundaryCondition>(bc_name, name, parameters);
  _fe_problem.theWarehouse().add(bc);

  const auto & boundary_ids = bc->boundaryIDs();
  const auto * const bc_var = bc->variable().mooseVar();
  _vars[0].addBoundaryVar(boundary_ids, bc_var);
}

void
LinearSystem::initialSetupKokkosLinearFV()
{
  // Kokkos objects are currently instantiated only for thread 0 (single copy per object).
  // All three queries below use AttribThread(0) for this reason. If Kokkos objects are ever
  // allowed to be multi-threaded, this function will need to loop over threads and each kernel's
  // initialSetup will work correctly via its own _tid member.
  std::vector<Moose::Kokkos::LinearFVElementalKernel *> elemental_kernels;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVElementalKernel")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .queryInto(elemental_kernels);

  for (auto * kernel : elemental_kernels)
    kernel->initialSetup();

  std::vector<Moose::Kokkos::LinearFVFluxKernel *> flux_kernels;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVFluxKernel")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .queryInto(flux_kernels);

  for (auto * kernel : flux_kernels)
    kernel->initialSetup();

  std::vector<Moose::Kokkos::LinearFVBoundaryCondition *> bcs;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVBoundaryCondition")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .queryInto(bcs);

  for (auto * bc : bcs)
    bc->initialSetup();
}

void
LinearSystem::computeKokkosLinearSystem(const std::set<TagID> & vector_tags,
                                        const std::set<TagID> & matrix_tags)
{
  TIME_SECTION("computeKokkosLinearSystem", 1);

  std::vector<Moose::Kokkos::LinearFVElementalKernel *> elemental_for_vector_tags;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVElementalKernel")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .template condition<AttribVectorTags>(vector_tags)
      .queryInto(elemental_for_vector_tags);

  std::vector<Moose::Kokkos::LinearFVElementalKernel *> elemental_for_matrix_tags;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVElementalKernel")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .template condition<AttribMatrixTags>(matrix_tags)
      .queryInto(elemental_for_matrix_tags);

  std::vector<Moose::Kokkos::LinearFVFluxKernel *> flux_for_vector_tags;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVFluxKernel")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .template condition<AttribVectorTags>(vector_tags)
      .queryInto(flux_for_vector_tags);

  std::vector<Moose::Kokkos::LinearFVFluxKernel *> flux_for_matrix_tags;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVFluxKernel")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .template condition<AttribMatrixTags>(matrix_tags)
      .queryInto(flux_for_matrix_tags);

  std::vector<Moose::Kokkos::LinearFVBoundaryCondition *> bcs_for_vector_tags;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVBoundaryCondition")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .template condition<AttribVectorTags>(vector_tags)
      .queryInto(bcs_for_vector_tags);

  std::vector<Moose::Kokkos::LinearFVBoundaryCondition *> bcs_for_matrix_tags;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVBoundaryCondition")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .template condition<AttribMatrixTags>(matrix_tags)
      .queryInto(bcs_for_matrix_tags);

  std::vector<Moose::Kokkos::LinearFVElementalKernel *> elemental_kernels;
  MooseUtils::getUnion(elemental_for_vector_tags, elemental_for_matrix_tags, elemental_kernels);

  std::vector<Moose::Kokkos::LinearFVFluxKernel *> flux_kernels;
  MooseUtils::getUnion(flux_for_vector_tags, flux_for_matrix_tags, flux_kernels);

  std::vector<Moose::Kokkos::LinearFVBoundaryCondition *> bcs;
  MooseUtils::getUnion(bcs_for_vector_tags, bcs_for_matrix_tags, bcs);

  if (elemental_kernels.empty() && flux_kernels.empty() && bcs.empty())
    return;

  auto & our_kokkos_system = _fe_problem.getKokkosSystem(number());
  our_kokkos_system.setActiveResidualTags(vector_tags);
  our_kokkos_system.setActiveMatrixTags(matrix_tags);

  std::set<TagID> needed_solution_vector_tags;

  for (const auto & tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_solution_vector_tags.insert(tag._id);

  auto & kokkos_systems = _fe_problem.getKokkosSystems();
  for (auto & system : kokkos_systems.constructedEntries())
  {
    system.setActiveSolutionTags(needed_solution_vector_tags);
    system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);
  }

  kokkos_systems.copyToDevice();

  for (auto bc : bcs_for_matrix_tags)
    bc->computeMatrix();
  for (auto kernel : elemental_for_matrix_tags)
    kernel->computeMatrix();
  for (auto kernel : flux_for_matrix_tags)
    kernel->computeMatrix();
  for (auto bc : bcs_for_vector_tags)
    bc->computeRightHandSide();
  for (auto kernel : elemental_for_vector_tags)
    kernel->computeRightHandSide();
  for (auto kernel : flux_for_vector_tags)
    kernel->computeRightHandSide();

  for (auto & system : kokkos_systems.constructedEntries())
    system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

  our_kokkos_system.clearActiveResidualTags();
  our_kokkos_system.clearActiveMatrixTags();

  for (auto & system : kokkos_systems.constructedEntries())
  {
    system.clearActiveVariables();
    system.clearActiveSolutionTags();
  }
}
