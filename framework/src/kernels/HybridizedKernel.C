//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HybridizedKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"
#include "NonlinearThread.h"
#include "AuxiliarySystem.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

InputParameters
HybridizedKernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  params.registerBase("HybridizedKernel");
  params.suppressParameter<std::vector<AuxVariableName>>("save_in");
  params.suppressParameter<std::vector<AuxVariableName>>("diag_save_in");
  return params;
}

const std::string HybridizedKernel::lm_increment_vector_name = "hybrid_lm_increment";

HybridizedKernel::HybridizedKernel(const InputParameters & parameters)
  : KernelBase(parameters),
    _aux_sys(_fe_problem.getAuxiliarySystem()),
    _lm_increment(nullptr),
    _current_bnd_id(_assembly.currentBoundaryID()),
    _qrule_face(_assembly.qRuleFace()),
    _q_point_face(_assembly.qPointsFace()),
    _JxW_face(_assembly.JxWFace()),
    _coord_face(_assembly.coordTransformation()),
    _normals(_assembly.normals()),
    _neigh(nullptr),
#ifndef NDEBUG
    _current_side(_assembly.side()),
#endif
    _computing_global_data(true)
{
}

void
HybridizedKernel::assemble()
{
  onElement();

  for (const auto side : _current_elem->side_index_range())
  {
    // Whether we have reinitialized the finite element (libMesh FE) objects on the current face
    bool fe_face_reinitd = false;

    const auto & boundary_ids = _mesh.getBoundaryIDs(_current_elem, side);
    for (const auto bnd_id : boundary_ids)
    {
      _fe_problem.setCurrentBoundaryID(bnd_id, _tid);
      if (fe_face_reinitd)
        // Only need to compute boundary materials; face materials have already been computed on the
        // current face
        _fe_problem.reinitMaterialsBoundary(_current_bnd_id, _tid);
      else
      {
        // Do the whole shebang:
        // - reinit FE objects on the face
        // - compute variable values on the face
        // - compute material values, including values from face and boundary materials
        NonlinearThread::prepareFace(_fe_problem, _tid, _current_elem, side, _current_bnd_id);
        fe_face_reinitd = true;
      }
      libmesh_assert(_current_side == side);
      onBoundary();
    }

    if (_neigh = _current_elem->neighbor_ptr(side); _neigh)
    {
      if (!fe_face_reinitd)
        NonlinearThread::prepareFace(
            _fe_problem, _tid, _current_elem, side, Moose::INVALID_BOUNDARY_ID);

      libmesh_assert(_current_side == side);
      onInternalSide();
    }
  }
}
