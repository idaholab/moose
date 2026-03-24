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
#include "KokkosLinearFVKernel.h"
#include "KokkosLinearFVBoundaryCondition.h"
#include "MooseUtils.h"
#include "Coupleable.h"
#include "MaterialPropertyInterface.h"
#include "MooseVariableDependencyInterface.h"
#include "MaterialBase.h"

#include "FEProblemBase.h"
#include "MooseVariableFieldBase.h"

namespace
{
template <typename T>
void
updateDependencies(const std::vector<T *> & objects,
                   std::set<MooseVariableFieldBase *> & needed_moose_vars,
                   std::set<TagID> & needed_fe_var_vector_tags,
                   std::unordered_set<unsigned int> & needed_mat_props)
{
  for (const auto * object : objects)
  {
    if (const auto * mvdi = dynamic_cast<const MooseVariableDependencyInterface *>(object))
    {
      const auto & mv_deps = mvdi->getMooseVariableDependencies();
      needed_moose_vars.insert(mv_deps.begin(), mv_deps.end());
    }

    if (const auto * coupleable = dynamic_cast<const Coupleable *>(object))
    {
      const auto & tag_deps = coupleable->getFEVariableCoupleableVectorTags();
      needed_fe_var_vector_tags.insert(tag_deps.begin(), tag_deps.end());
    }

    if (const auto * mpi = dynamic_cast<const MaterialPropertyInterface *>(object))
    {
      const auto & mp_deps = mpi->getMatPropDependencies();
      needed_mat_props.insert(mp_deps.begin(), mp_deps.end());
    }
  }
}
}

void
LinearSystem::addKokkosKernel(const std::string & kernel_name,
                              const std::string & name,
                              InputParameters & parameters)
{
  auto kernel = _factory.create<Moose::Kokkos::LinearFVKernel>(kernel_name, name, parameters);

  if (std::dynamic_pointer_cast<Moose::Kokkos::LinearFVElementalKernel>(kernel))
    _fe_problem.theWarehouse().add(kernel);
  else if (std::dynamic_pointer_cast<Moose::Kokkos::LinearFVFluxKernel>(kernel))
    _fe_problem.theWarehouse().add(kernel);
  else
    mooseError("Unknown Kokkos LinearFVKernel type for object named ", kernel->name());
}

void
LinearSystem::addKokkosBoundaryCondition(const std::string & bc_name,
                                         const std::string & name,
                                         InputParameters & parameters)
{
  auto bc = _factory.create<Moose::Kokkos::LinearFVBoundaryCondition>(bc_name, name, parameters);
  _fe_problem.theWarehouse().add(bc);

  const auto & boundary_ids = bc->boundaryIDs();
  auto bc_var = dynamic_cast<const MooseVariableFieldBase *>(&bc->variable());
  _vars[0].addBoundaryVar(boundary_ids, bc_var);
}

void
LinearSystem::initialSetupKokkosLinearFV()
{
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

  std::vector<Moose::Kokkos::LinearFVElementalKernel *> elemental_after_vectors;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVElementalKernel")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .template condition<AttribVectorTags>(vector_tags)
      .queryInto(elemental_after_vectors);

  std::vector<Moose::Kokkos::LinearFVElementalKernel *> elemental_after_matrices;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVElementalKernel")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .template condition<AttribMatrixTags>(matrix_tags)
      .queryInto(elemental_after_matrices);

  std::vector<Moose::Kokkos::LinearFVFluxKernel *> flux_after_vectors;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVFluxKernel")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .template condition<AttribVectorTags>(vector_tags)
      .queryInto(flux_after_vectors);

  std::vector<Moose::Kokkos::LinearFVFluxKernel *> flux_after_matrices;
  _fe_problem.theWarehouse()
      .query()
      .template condition<AttribSysNum>(number())
      .template condition<AttribSystem>("LinearFVFluxKernel")
      .template condition<AttribKokkos>(true)
      .template condition<AttribThread>(0)
      .template condition<AttribMatrixTags>(matrix_tags)
      .queryInto(flux_after_matrices);

  std::vector<Moose::Kokkos::LinearFVElementalKernel *> elemental_kernels;
  MooseUtils::getUnion(elemental_after_vectors, elemental_after_matrices, elemental_kernels);

  std::vector<Moose::Kokkos::LinearFVFluxKernel *> flux_kernels;
  MooseUtils::getUnion(flux_after_vectors, flux_after_matrices, flux_kernels);

  if (elemental_kernels.empty() && flux_kernels.empty())
    return;

  auto & systems = _fe_problem.getKokkosSystems();
  systems[number()].setActiveResidualTags(vector_tags);
  systems[number()].setActiveMatrixTags(matrix_tags);

  std::set<MooseVariableFieldBase *> needed_moose_vars;
  std::set<TagID> needed_fe_var_vector_tags;
  std::unordered_set<unsigned int> needed_mat_props;

  for (const auto & tag : _fe_problem.getVectorTags(Moose::VECTOR_TAG_SOLUTION))
    needed_fe_var_vector_tags.insert(tag._id);

  updateDependencies(
      elemental_kernels, needed_moose_vars, needed_fe_var_vector_tags, needed_mat_props);
  updateDependencies(flux_kernels, needed_moose_vars, needed_fe_var_vector_tags, needed_mat_props);

  if (needed_mat_props.size())
  {
    _fe_problem.getKokkosMaterialsWarehouse().updateVariableDependency(needed_moose_vars);
    _fe_problem.getKokkosMaterialsWarehouse().updateFEVariableCoupledVectorTagDependency(
        needed_fe_var_vector_tags);
  }

  for (auto & system : systems)
  {
    system.setActiveVariables(needed_moose_vars);
    system.setActiveSolutionTags(needed_fe_var_vector_tags);
    system.sync(Moose::Kokkos::MemcpyType::HOST_TO_DEVICE);
    system.reinit();
  }

  systems.copyToDevice();

  if (needed_mat_props.size())
  {
    _fe_problem.prepareKokkosMaterials(needed_mat_props);
    _fe_problem.reinitKokkosMaterials();
  }

  for (auto kernel : elemental_after_matrices)
    kernel->computeMatrix();
  for (auto kernel : flux_after_matrices)
    kernel->computeMatrix();
  for (auto kernel : elemental_after_vectors)
    kernel->computeRightHandSide();
  for (auto kernel : flux_after_vectors)
    kernel->computeRightHandSide();

  for (auto & system : systems)
    system.sync(Moose::Kokkos::MemcpyType::DEVICE_TO_HOST);

  systems[number()].clearActiveResidualTags();
  systems[number()].clearActiveMatrixTags();

  for (auto & system : systems)
  {
    system.clearActiveVariables();
    system.clearActiveSolutionTags();
  }
}
