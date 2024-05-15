//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HDGKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"
#include "NonlinearThread.h"
#include "AuxiliarySystem.h"
#include "HDGIntegratedBC.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

InputParameters
HDGKernel::validParams()
{
  InputParameters params = KernelBase::validParams();
  params.registerBase("HDGKernel");
  params.suppressParameter<std::vector<AuxVariableName>>("save_in");
  params.suppressParameter<std::vector<AuxVariableName>>("diag_save_in");
  params.addPrivateParam<MooseObjectWarehouse<HDGIntegratedBC> *>("hibc_warehouse");
  return params;
}

const std::string HDGKernel::lm_increment_vector_name = "hybrid_lm_increment";

HDGKernel::HDGKernel(const InputParameters & parameters)
  : KernelBase(parameters),
    ADFunctorInterface(this),
    _aux_sys(_fe_problem.getAuxiliarySystem()),
    _lm_increment(nullptr),
    _current_bnd_id(_assembly.currentBoundaryID()),
    _qrule_face(_assembly.qRuleFace()),
    _q_point_face(_assembly.qPointsFace()),
    _JxW_face(_assembly.JxWFace()),
    _coord_face(_assembly.coordTransformation()),
    _normals(_assembly.normals()),
    _neigh(nullptr),
    _current_side(_assembly.side()),
    _preparing_for_solve(true),
    _hibc_warehouse(
        *getCheckedPointerParam<MooseObjectWarehouse<HDGIntegratedBC> *>("hibc_warehouse"))
{
}

void
HDGKernel::addBCData(const HDGIntegratedBC & hibc)
{
  _PrimalMat += hibc._PrimalMat;
  _LMMat += hibc._LMMat;
  _PrimalLM += hibc._PrimalLM;
  _LMPrimal += hibc._LMPrimal;
  _PrimalVec += hibc._PrimalVec;
  _LMVec += hibc._LMVec;
}

void
HDGKernel::assemble()
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

        mooseAssert(_current_side == side, "The sides should be the same");
        const auto & bcs = _hibc_warehouse.getActiveBoundaryObjects(bnd_id, _tid);
        for (const auto & bc : bcs)
        {
          mooseAssert(bc->shouldApply(),
                      "I don't think anyone uses the shouldApply feature for integrated boundary "
                      "conditions");
          bc->onBoundary();
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

      mooseAssert(_current_side == side, "The sides should be the same");
      onInternalSide();
    }
  }

  //
  // We've now completed our local finite element assembly. We now add this local data into the
  // global data structures. When preparing for the linear solve, which only occurs for the Lagrange
  // multiplier degrees of freedom, we fill the global residual and Jacobian (this is during the
  // ComputeResidualAndJacobianThread). Else we are in the post-linear-solve stage when we already
  // have already compute our Lagrange multiplier increment, and with that we can compute the primal
  // solution increment and add it into the auxiliary system vector which holds the primal degrees
  // of freedom (this is during the HDGPrimalSolutionUpdateThread)
  //

  _PrimalMatInv = _PrimalMat.inverse();
  const auto lm_size = _lm_dof_indices.size();
  if (_preparing_for_solve)
  {
    _K_libmesh.resize(lm_size, lm_size);
    _F_libmesh.resize(lm_size);
    const auto LMProductMat = -_LMPrimal * _PrimalMatInv;
    _LMMat += LMProductMat * _PrimalLM;
    _LMVec += LMProductMat * _PrimalVec;
    mooseAssert(cast_int<std::size_t>(_LMMat.rows()) == lm_size,
                "The number of on-diagonal LM matrix rows should match the number of LM degrees of "
                "freedom");
    mooseAssert(cast_int<std::size_t>(_LMMat.cols()) == lm_size,
                "The number of on-diagonal LM matrix columns should match the number of LM degrees "
                "of freedom");
    mooseAssert(cast_int<std::size_t>(_LMVec.size()) == lm_size,
                "The size of the LM vector should match the number of LM degrees of freedom");

    for (const auto i : make_range(lm_size))
    {
      for (const auto j : make_range(lm_size))
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
    _LMIncrement.resize(lm_size);
    for (const auto i : index_range(_lm_dof_indices))
      _LMIncrement(i) = _lm_increment_dof_values[i];

    _PrimalIncrement = _PrimalMatInv * (-_PrimalVec - _PrimalLM * _LMIncrement);
    _primal_increment_dof_values.resize(_primal_dof_indices.size());
    for (const auto i : index_range(_primal_increment_dof_values))
      _primal_increment_dof_values[i] = _PrimalIncrement(i);

    _aux_sys.solution().add_vector(_primal_increment_dof_values, _primal_dof_indices);
  }
}

void
HDGKernel::initialSetup()
{
  const auto our_physics = physics();
  const auto & our_vars = variables();
  std::set<std::string> our_var_names, their_var_names;
  for (const auto * const our_var : our_vars)
  {
    our_var_names.insert(our_var->name());
    if (!MooseUtils::relativeFuzzyEqual(our_var->scalingFactor(), Real(1)))
      mooseError("Scaling factors disrupt the relationship between Lagrange multiplier and primal "
                 "solution increments. '",
                 our_var->name(),
                 "' has a non-unity scaling factor");
  }

  const auto & bcs = _hibc_warehouse.getObjects(_tid);
  for (const auto & bc : bcs)
  {
    if (bc->physics() != our_physics)
      mooseError("'",
                 bc->name(),
                 "' implements '",
                 bc->physics(),
                 "' physics, which doesn't match our '",
                 our_physics,
                 "' physics");

    const auto & their_vars = bc->variables();
    their_var_names.clear();
    for (const auto * const their_var : their_vars)
      their_var_names.insert(their_var->name());

    if (bc->variables() != our_vars)
      mooseError("Different variables are being used in the boundary condition '",
                 bc->name(),
                 "'. HDG kernels and boundary conditions must all operate on the same variables");
  }

  KernelBase::initialSetup();
  _lm_increment = &_sys.getVector(lm_increment_vector_name);
}
