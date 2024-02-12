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
#include "HybridizedIntegratedBC.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

InputParameters
HybridizedKernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  params.registerBase("HybridizedKernel");
  params.suppressParameter<std::vector<AuxVariableName>>("save_in");
  params.suppressParameter<std::vector<AuxVariableName>>("diag_save_in");
  params.addPrivateParam<MooseObjectWarehouse<HybridizedIntegratedBC> *>("hibc_warehouse");
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
    _computing_global_data(true),
    _hibc_warehouse(
        *getCheckedPointerParam<MooseObjectWarehouse<HybridizedIntegratedBC> *>("hibc_warehouse"))
{
  // This class handles residuals and Jacobians for multiple variables
  _fe_problem.setKernelCoverageCheck(false);
}

void
HybridizedKernel::addBCData(const HybridizedIntegratedBC & hibc)
{
  _PrimalMat += hibc._PrimalMat;
  _LMMat += hibc._LMMat;
  _PrimalLM += hibc._PrimalLM;
  _LMPrimal += hibc._LMPrimal;
  _PrimalVec += hibc._PrimalVec;
  _LMVec += hibc._LMVec;
}

void
HybridizedKernel::assemble()
{
  onElement();

  for (const auto side : _current_elem->side_index_range())
  {
    // Whether we have reinitialized the finite element (libMesh FE) objects on the current face
    bool fe_face_reinitd = false;

    // Set up Sentinel class so that, even if reinitMaterialsFace() throws, we
    // still remember to swap back during stack unwinding. Stack is LIFO so this will be destructed
    // before fe_face_reinitd, so it's valid for us to pass it in here
    SwapBackSentinel sentinel(
        _fe_problem, &FEProblemBase::swapBackMaterialsFace, _tid, fe_face_reinitd, false);

    const auto & boundary_ids = _mesh.getBoundaryIDs(_current_elem, side);
    for (const auto bnd_id : boundary_ids)
      if (_hibc_warehouse.hasActiveBoundaryObjects(bnd_id, _tid))
      {
        _fe_problem.setCurrentBoundaryID(bnd_id, _tid);

        if (!fe_face_reinitd)
        {
          fe_face_reinitd = true;
          // Do the whole shebang:
          // - reinit FE objects on the face
          // - compute variable values on the face
          // - compute material values, including values from face and boundary materials
          NonlinearThread::prepareFace(_fe_problem, _tid, _current_elem, side, _current_bnd_id);
        }
        else
          // Only need to compute boundary materials; face materials have already been computed on
          // the current face
          _fe_problem.reinitMaterialsBoundary(_current_bnd_id, _tid);

        libmesh_assert(_current_side == side);
        const auto & bcs = _hibc_warehouse.getActiveBoundaryObjects(bnd_id, _tid);
        for (const auto & bc : bcs)
        {
          mooseAssert(bc->shouldApply(),
                      "I don't think anyone uses the shouldApply feature for integrated boundary "
                      "conditions");
          bc->assemble();
          addBCData(*bc);
        }
      }

    if (_neigh = _current_elem->neighbor_ptr(side);
        _neigh && this->hasBlocks(_neigh->subdomain_id()))
    {
      if (!fe_face_reinitd)
      {
        fe_face_reinitd = true;
        NonlinearThread::prepareFace(
            _fe_problem, _tid, _current_elem, side, Moose::INVALID_BOUNDARY_ID);
      }

      libmesh_assert(_current_side == side);
      onInternalSide();
    }
  }

  _PrimalMatInv = _PrimalMat.inverse();
  libmesh_assert(_lm_size == _lm_dof_indices.size());
  if (_computing_global_data)
  {
    _K_libmesh.resize(_lm_size, _lm_size);
    _F_libmesh.resize(_lm_size);
    const auto LMProductMat = -_LMPrimal * _PrimalMatInv;
    _LMMat += LMProductMat * _PrimalLM;
    _LMVec += LMProductMat * _PrimalVec;
    libmesh_assert(cast_int<std::size_t>(_LMMat.rows()) == _lm_size);
    libmesh_assert(cast_int<std::size_t>(_LMMat.cols()) == _lm_size);
    libmesh_assert(cast_int<std::size_t>(_LMVec.size()) == _lm_size);

    for (const auto i : make_range(_lm_size))
    {
      for (const auto j : make_range(_lm_size))
        _K_libmesh(i, j) = _LMMat(i, j);
      _F_libmesh(i) = _LMVec(i);
    }

    // We were performing our finite element assembly for the implicit solve step of our
    // example. Add our local element vectors/matrices into the global system
    addResiduals(_assembly, _F_libmesh, _lm_dof_indices, /*scaling_factor=*/1);
    addJacobian(_assembly, _K_libmesh, _lm_dof_indices, _lm_dof_indices, /*scaling_factor=*/1);
  }
  else
  {
    //
    // We are doing our finite element assembly for the second time. We now know the Lagrange
    // multiplier solution. With that and the local element matrices and vectors we can compute
    // the vector and scalar solutions
    //
    _lm_increment->get(_lm_dof_indices, _lm_increment_dof_values);
    _LMIncrement.resize(_lm_size);
    for (const auto i : index_range(_lm_dof_indices))
      _LMIncrement(i) = _lm_increment_dof_values[i];

    _PrimalIncrement = _PrimalMatInv * (-_PrimalVec - _PrimalLM * _LMIncrement);
    libmesh_assert(_primal_dof_indices.size() == _primal_size);
    _primal_increment_dof_values.resize(_primal_size);
    for (const auto i : index_range(_primal_increment_dof_values))
      _primal_increment_dof_values[i] = _PrimalIncrement(i);

    _aux_sys.solution().add_vector(_primal_increment_dof_values, _primal_dof_indices);
  }
}
