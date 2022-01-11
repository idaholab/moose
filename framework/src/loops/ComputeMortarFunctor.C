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
    bool displaced)
  : _amg(amg),
    _subproblem(subproblem),
    _fe_problem(fe_problem),
    _displaced(displaced),
    _assembly(_subproblem.assembly(0))
{
  // Construct the mortar constraints we will later loop over
  for (auto mc : mortar_constraints)
    _mortar_constraints.push_back(mc.get());
}

void
ComputeMortarFunctor::operator()()
{
  unsigned int num_cached = 0;

  // Compile map of secondary to msm elements
  // All this could be computed in AutomaticMortarGeneration, will suffice for now but
  // in the future may want to move for optimization, especially on non-displaced meshes
  std::map<const Elem *, std::vector<Elem *>> secondary_elems_to_mortar_segments;

  for (const auto msm_elem : _amg.mortarSegmentMesh().active_local_element_ptr_range())
  {
    const MortarSegmentInfo & msinfo =
        libmesh_map_find(_amg.mortarSegmentMeshElemToInfo(), msm_elem);
    const Elem * secondary_face_elem = msinfo.secondary_elem;

    // Compute fraction of volume of secondary element the segment accounts for
    const Real volume_fraction = msm_elem->volume() / secondary_face_elem->volume();

    // Neglect small segments to avoid negative Jacobian errors for 0 volume segments
    if (volume_fraction < TOLERANCE)
      continue;

    secondary_elems_to_mortar_segments[secondary_face_elem].push_back(msm_elem);
  }

  auto act_functor = [this, &num_cached]()
  {
    ++num_cached;
    if (!_fe_problem.currentlyComputingJacobian())
    {
      for (auto * const mc : _mortar_constraints)
        mc->computeResidual();

      _assembly.cacheResidual();
      _assembly.cacheResidualNeighbor();
      _assembly.cacheResidualLower();

      if (num_cached % 20 == 0)
        _assembly.addCachedResiduals();
    }
    else
    {
      for (auto * const mc : _mortar_constraints)
        mc->computeJacobian();

      _assembly.cacheJacobianMortar();

      if (num_cached % 20 == 0)
        _assembly.addCachedJacobian();
    }
  };

  Moose::Mortar::loopOverMortarSegments(secondary_elems_to_mortar_segments,
                                        _assembly,
                                        _subproblem,
                                        _fe_problem,
                                        _amg,
                                        _displaced,
                                        _mortar_constraints,
                                        act_functor);

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
  if (!_fe_problem.currentlyComputingJacobian())
    _assembly.addCachedResiduals();
  else
    _assembly.addCachedJacobian();
}
