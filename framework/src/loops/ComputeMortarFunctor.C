//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMortarFunctor.h"
#include "FEProblemBase.h"
#include "SubProblem.h"
#include "Assembly.h"
#include "ADMortarConstraint.h"
#include "AutomaticMortarGeneration.h"
#include "MooseMesh.h"
#include "Assembly.h"
#include "MortarUtils.h"
#include "MaterialBase.h"

#include "libmesh/fe_base.h"
#include "libmesh/quadrature.h"
#include "libmesh/elem.h"
#include "libmesh/point.h"
#include "libmesh/mesh_base.h"

ComputeMortarFunctor::ComputeMortarFunctor(
    const std::vector<std::shared_ptr<MortarConstraintBase>> & mortar_constraints,
    const AutomaticMortarGeneration & amg,
    SubProblem & subproblem,
    FEProblemBase & fe_problem,
    bool displaced,
    Assembly & assembly)
  : _amg(amg),
    _subproblem(subproblem),
    _fe_problem(fe_problem),
    _displaced(displaced),
    _assembly(assembly)
{
  // Construct the mortar constraints we will later loop over
  for (auto mc : mortar_constraints)
    _mortar_constraints.push_back(mc.get());

  Moose::Mortar::setupMortarMaterials(_mortar_constraints,
                                      _fe_problem,
                                      _amg,
                                      0,
                                      _secondary_ip_sub_to_mats,
                                      _primary_ip_sub_to_mats,
                                      _secondary_boundary_mats);
}

void
ComputeMortarFunctor::operator()(const Moose::ComputeType compute_type,
                                 const std::set<TagID> & vector_tag_ids,
                                 const std::set<TagID> & /*matrix_tag_ids*/)
{
  libmesh_parallel_only(_fe_problem.comm());

  unsigned int num_cached = 0;
  const auto & vector_tags = _fe_problem.getVectorTags(vector_tag_ids);

  const auto & secondary_elems_to_mortar_segments = _amg.secondariesToMortarSegments();
  typedef decltype(secondary_elems_to_mortar_segments.begin()) it_type;

  std::vector<it_type> iterators;
  for (auto it = secondary_elems_to_mortar_segments.begin();
       it != secondary_elems_to_mortar_segments.end();
       ++it)
  {
    auto * const secondary_elem = _subproblem.mesh().getMesh().query_elem_ptr(it->first);

    if (secondary_elem && secondary_elem->processor_id() == _subproblem.processor_id() &&
        !it->second.empty())
    {
      // This is local and the mortar segment set isn't empty, so include
      iterators.push_back(it);
      mooseAssert(secondary_elem->active(),
                  "We loop over active elements when building the mortar segment mesh, so we golly "
                  "well hope this is active.");
    }
  }

  auto act_functor = [this, &num_cached, compute_type, &vector_tags]()
  {
    ++num_cached;

    switch (compute_type)
    {
      case Moose::ComputeType::Residual:
      {
        for (auto * const mc : _mortar_constraints)
        {
          mc->setNormals();
          mc->computeResidual();
        }

        _assembly.cacheResidual(Assembly::GlobalDataKey{}, vector_tags);
        _assembly.cacheResidualNeighbor(Assembly::GlobalDataKey{}, vector_tags);
        _assembly.cacheResidualLower(Assembly::GlobalDataKey{}, vector_tags);

        if (num_cached % 20 == 0)
          _assembly.addCachedResiduals(Assembly::GlobalDataKey{}, vector_tags);

        break;
      }

      case Moose::ComputeType::Jacobian:
      {
        for (auto * const mc : _mortar_constraints)
        {
          mc->setNormals();
          mc->computeJacobian();
        }

        _assembly.cacheJacobianMortar(Assembly::GlobalDataKey{});

        if (num_cached % 20 == 0)
          _assembly.addCachedJacobian(Assembly::GlobalDataKey{});
        break;
      }

      case Moose::ComputeType::ResidualAndJacobian:
      {
        for (auto * const mc : _mortar_constraints)
        {
          mc->setNormals();
          mc->computeResidualAndJacobian();
        }

        _assembly.cacheResidual(Assembly::GlobalDataKey{}, vector_tags);
        _assembly.cacheResidualNeighbor(Assembly::GlobalDataKey{}, vector_tags);
        _assembly.cacheResidualLower(Assembly::GlobalDataKey{}, vector_tags);
        _assembly.cacheJacobianMortar(Assembly::GlobalDataKey{});

        if (num_cached % 20 == 0)
        {
          _assembly.addCachedResiduals(Assembly::GlobalDataKey{}, vector_tags);
          _assembly.addCachedJacobian(Assembly::GlobalDataKey{});
        }
        break;
      }
    }
  };

  PARALLEL_TRY
  {
    try
    {
      Moose::Mortar::loopOverMortarSegments(iterators,
                                            _assembly,
                                            _subproblem,
                                            _fe_problem,
                                            _amg,
                                            _displaced,
                                            _mortar_constraints,
                                            0,
                                            _secondary_ip_sub_to_mats,
                                            _primary_ip_sub_to_mats,
                                            _secondary_boundary_mats,
                                            act_functor,
                                            /*reinit_mortar_user_objects=*/true);
    }
    catch (libMesh::LogicError & e)
    {
      _fe_problem.setException("We caught a libMesh::LogicError: " + std::string(e.what()));
    }
    catch (MooseException & e)
    {
      _fe_problem.setException(e.what());
    }
    catch (MetaPhysicL::LogicError & e)
    {
      moose::translateMetaPhysicLError(e);
    }
  }
  PARALLEL_CATCH;

  // Call any post operations for our mortar constraints
  for (auto * const mc : _mortar_constraints)
  {
    if (_amg.incorrectEdgeDropping())
      mc->incorrectEdgeDroppingPost(_amg.getInactiveLMNodes());
    else
      mc->post();

    mc->zeroInactiveLMDofs(_amg.getInactiveLMNodes(), _amg.getInactiveLMElems());
  }

  // Make sure any remaining cached residuals/Jacobians get added
  if (_assembly.computingResidual())
    _assembly.addCachedResiduals(Assembly::GlobalDataKey{}, vector_tags);
  if (_assembly.computingJacobian())
    _assembly.addCachedJacobian(Assembly::GlobalDataKey{});
}
