//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RigidBodyLoadControl.h"

#include "Assembly.h"
#include "AuxiliarySystem.h"
#include "FEProblemBase.h"
#include "Function.h"
#include "LevelSetContactor.h"
#include "MooseMesh.h"
#include "MooseVariable.h"
#include "MooseVariableScalar.h"
#include "NodalArea.h"
#include "SystemBase.h"

registerMooseObject("ContactApp", RigidBodyLoadControl);

InputParameters
RigidBodyLoadControl::validParams()
{
  InputParameters params = NodalScalarKernel::validParams();
  params.addClassDescription("Load-control constraint for a rigid-body contactor: enforces "
                             "F(t) = Sum_i w_i * lambda_i * (n_i . direction) so that the scalar "
                             "variable driving the contactor's translation in `direction` is "
                             "determined by the target contact reaction.");
  // Parallel: MOOSE only calls ScalarKernel::computeResidual/Jacobian on
  // the rank owning the scalar DoF (which is the last MPI rank), so that
  // rank must see EVERY boundary node in `_node_ids` to correctly sum
  // the integrated reaction and populate the Jacobian blocks.  The
  // required GhostBoundary relationship manager is registered by the
  // companion `RigidBodyContactSparsity` UO -- it has to be there anyway
  // for the sparsity augmentation, so its GhostBoundary side effect
  // covers this kernel's parallel needs too.
  params.addRequiredParam<FunctionName>(
      "force", "Function returning the target integrated contact reaction F(t) along direction.");
  params.addRequiredParam<UserObjectName>(
      "contactor",
      "LevelSetContactor whose translation in `direction` is driven by this kernel's `variable`.");
  params.addRequiredParam<UserObjectName>(
      "nodal_area", "NodalArea UO providing tributary weights w_i on the contact sideset.");
  params.addRequiredCoupledVar(
      "lm_variable",
      "The Lagrange multiplier field variable (nodal, on the contact sideset's lower-d block).");
  params.addRequiredCoupledVar("displacements", "Displacement variables in order (x, y[, z]).");
  params.addRequiredParam<Point>(
      "direction",
      "Unit vector along which the integrated normal contact reaction is measured.  Must be "
      "aligned with a Cartesian axis and match the contactor axis driven by this kernel's "
      "`variable`.");
  params.addRangeCheckedParam<Real>(
      "c",
      1.0,
      "c > 0",
      "NCP scaling on the gap.  MUST match the `c` used by the companion "
      "RigidBodyNodalNCPKernel so the (LM_row, scalar_col) transpose Jacobian block is correct.");
  params.addRangeCheckedParam<Real>(
      "kss_stiffness",
      0.0,
      "kss_stiffness >= 0",
      "Constant scalar variant of the effective contact stiffness used to "
      "fabricate a nonzero (scalar_row, scalar_col) Jacobian entry as "
      "`Kss = kss_stiffness * total_nodal_area * sign(direction . axis_hat)`. "
      "This is a JACOBIAN-ONLY preconditioning modification (residual is "
      "unchanged, so the physical fixed point R_s = 0 is unchanged) that "
      "keeps Newton from overshooting when the true Schur-complement Kss is "
      "O(K_material * A_contact) but the assembled block is zero.  Set to 0 "
      "(default) to disable.  A sensible value is the deformable body's "
      "Young's modulus.  Also serves as the approximation of dR_s/ds used by "
      "`UzawaTransient` for its scalar Newton step.  Mutually exclusive with "
      "`kss_stiffness_function`.");
  params.addParam<FunctionName>(
      "kss_stiffness_function",
      "Function-of-time variant of the same effective contact stiffness. "
      "Evaluated at (current time, origin) whenever the stiffness is needed "
      "(computeJacobian, UzawaTransient's `signedKssApprox`), replacing the "
      "constant `kss_stiffness` for that assembly.  Useful when a single "
      "constant does not cover the range spanned by the load history -- "
      "e.g. much larger during initial impact than during a well-established "
      "plastic patch.  Mutually exclusive with `kss_stiffness`.");
  params.addParam<MooseEnum>(
      "mode",
      MooseEnum("ForceBalance PinScalar", "ForceBalance"),
      "Initial residual/Jacobian form.  ForceBalance (default) is the "
      "physical formulation `R_s = Sum w * lambda * (n.dir) - F(t)` with the "
      "Kls transpose and Ksl blocks and the optional `kss_stiffness` "
      "diagonal shift.  PinScalar emits `R_s = s - s_pin, Kss = 1` and no "
      "coupling blocks -- used by `UzawaTransient` during the primal solve "
      "to hold `s` fixed at the current outer-iterate value.  This param "
      "sets the *starting* mode; `UzawaTransient` calls `setMode()` at "
      "runtime to flip between the two.");
  params.addParam<Real>("s_pin",
                        0.0,
                        "Initial value of the PinScalar target.  Runtime updates go through "
                        "`setMode()`.  In ForceBalance mode this parameter is ignored.");
  return params;
}

RigidBodyLoadControl::RigidBodyLoadControl(const InputParameters & parameters)
  : NodalScalarKernel(parameters),
    _force(getFunction("force")),
    _contactor(getUserObject<LevelSetContactor>("contactor")),
    _nodal_area(getUserObject<NodalArea>("nodal_area")),
    _direction(getParam<Point>("direction")),
    _kss_stiffness_constant(getParam<Real>("kss_stiffness")),
    _kss_stiffness_function(
        isParamValid("kss_stiffness_function") ? &getFunction("kss_stiffness_function") : nullptr),
    _axis(libMesh::invalid_uint),
    _c(getParam<Real>("c")),
    _lm_var_num(coupled("lm_variable")),
    _lambda(coupledValue("lm_variable")),
    _ndisp(coupledComponents("displacements")),
    _disp_var_num(_ndisp),
    _disp(_ndisp),
    _mode(getParam<MooseEnum>("mode").getEnum<Mode>()),
    _s_pin(getParam<Real>("s_pin")),
    _cached_reaction_minus_F(0.0)
{
  if (_kss_stiffness_function && isParamSetByUser("kss_stiffness"))
    paramError("kss_stiffness_function",
               "Set exactly one of `kss_stiffness` (constant) or "
               "`kss_stiffness_function` (function of time), not both.");

  const Real n = _direction.norm();
  if (n < TOLERANCE)
    paramError("direction", "Must be a nonzero vector.");
  _direction /= n;

  // Verify the direction is aligned with a Cartesian axis, and that the
  // contactor's translation input on that axis is the scalar we are the
  // kernel of.  Anything else means the user's plumbing is inconsistent.
  for (const auto k : {0u, 1u, 2u})
    if (std::abs(std::abs(_direction(k)) - 1.0) < TOLERANCE)
      _axis = k;
  if (_axis == libMesh::invalid_uint)
    paramError("direction",
               "Must be aligned with a Cartesian axis (a signed unit vector on x, y, or z).");
  const unsigned int contactor_scalar = _contactor.translationScalarNumber(_axis);
  if (contactor_scalar == libMesh::invalid_uint)
    paramError("contactor",
               "The contactor's translation on axis ",
               _axis,
               " must be driven by a Scalar variable (via disp_",
               std::array<const char *, 3>{"x", "y", "z"}[_axis],
               "_scalar) for this kernel to close the load-control loop.");
  if (contactor_scalar != _var.number())
    paramError("variable",
               "This kernel's `variable` (number ",
               _var.number(),
               ") must match the Scalar variable driving the contactor's axis-",
               _axis,
               " translation (number ",
               contactor_scalar,
               ").");

  for (const auto k : make_range(_ndisp))
  {
    _disp[k] = &coupledValue("displacements", k);
    _disp_var_num[k] = coupled("displacements", k);
  }
}

Point
RigidBodyLoadControl::deformedNode(std::size_t k) const
{
  const Node & node = _mesh.getMesh().node_ref(_node_ids[k]);
  Point x = node;
  for (const auto d : make_range(_ndisp))
    x(d) += (*_disp[d])[k];
  return x;
}

Point
RigidBodyLoadControl::deformedNodePoint(const Node & node, std::size_t compressed) const
{
  // Compressed = index into `_lambda` / `_disp` vectors (only accessible
  // + semi-local nodes were stored, in _node_ids order).
  Point x = node;
  for (const auto d : make_range(_ndisp))
    x(d) += (*_disp[d])[compressed];
  return x;
}

bool
RigidBodyLoadControl::nodeIsAccessible(std::size_t k) const
{
  return _mesh.getMesh().query_node_ptr(_node_ids[k]) != nullptr;
}

void
RigidBodyLoadControl::setMode(Mode m, Real s_pin)
{
  _mode = m;
  _s_pin = s_pin;
}

Real
RigidBodyLoadControl::currentReactionMinusF() const
{
  // Sum reduction across all ranks: the value was populated only on
  // the scalar-owning rank (in computeResidual); other ranks
  // contribute 0.  Result is the correct scalar residual on every
  // rank.
  Real reduced = _cached_reaction_minus_F;
  _communicator.sum(reduced);
  return reduced;
}

void
RigidBodyLoadControl::computeResidual()
{
  // Parallel note: MOOSE calls ScalarKernel::computeResidual only on the
  // rank that owns the scalar DoF (NonlinearSystemBase.C ~line 1846), so
  // this method is single-rank and cannot do MPI-collective reductions.
  // The owning rank sees every boundary node in `_node_ids` (populated
  // globally in NodalScalarKernel), and by way of MOOSE's ghosting the
  // ones it does not own directly are still accessible via query_node_ptr
  // -- we still guard with a null check for robustness against unusual
  // partitions.  `_lambda[k]` and `_disp[k]` at those ghosted nodes carry
  // the correct up-to-date values because `NodalScalarKernel::reinit()`
  // called `_subproblem.reinitNodes(_node_ids, _tid)` beforehand.
  //
  // The reaction sum is ALWAYS computed (both modes need it: ForceBalance
  // to emit it as R_s, PinScalar to cache it for UzawaTransient's outer
  // Newton step to read via currentReactionMinusF()).  Only the value
  // written into `_local_re(0)` differs.
  const Real F = _force.value(_t, Point());
  Real reaction = 0.0;
  // `_lambda` and `_disp[k]` are populated by NodalScalarKernel::reinit()
  // via reinitNodes, which pushes one entry per node in `_node_ids` that
  // is accessible AND semi-local (owned or ghosted) on this rank.  In
  // parallel the compressed count may be smaller than `_node_ids.size()`,
  // so we can't simply index `_lambda[k]` with the full-list `k` --
  // instead iterate a running compressed index over accessible nodes,
  // matching the order MOOSE used internally.
  std::size_t j = 0;
  for (const auto k : index_range(_node_ids))
  {
    if (!nodeIsAccessible(k))
      continue;
    const Node * node = _mesh.getMesh().node_ptr(_node_ids[k]);
    const Real w = _nodal_area.nodalArea(node);
    const auto q = _contactor.queryAt(deformedNodePoint(*node, j));
    reaction += w * _lambda[j] * (q.normal * _direction);
    ++j;
  }
  _cached_reaction_minus_F = reaction - F;

  prepareVectorTag(_assembly, _var.number());
  if (_mode == Mode::PinScalar)
    // R_s = s - s_pin.  `_u[0]` is the current scalar variable value.
    _local_re(0) = _u[0] - _s_pin;
  else
    _local_re(0) = _cached_reaction_minus_F;
  assignTaggedLocalResidual();
}

void
RigidBodyLoadControl::computeJacobian()
{
  // PinScalar mode: R_s = s - s_pin.  Emit Kss = 1 and stop.  We do NOT
  // touch the (s, lambda) or (lambda, s) blocks: with Kss = 1 dominating
  // the scalar row, Newton drives `s -> s_pin` immediately at iter 0 and
  // the (u, lambda) primal solve then decouples.  Leaving Kls / Ksl
  // stale in those blocks would just add a tiny irrelevant coupling that
  // the outer SNES trivially handles.
  if (_mode == Mode::PinScalar)
  {
    prepareMatrixTag(_assembly, _var.number(), _var.number());
    for (const auto i : make_range(_local_ke.m()))
      for (const auto j : make_range(_local_ke.n()))
        _local_ke(i, j) = 0.0;
    _local_ke(0, 0) = 1.0;
    assignTaggedLocalMatrix();
    // Zero the (scalar_row, lambda_col) block so previous ForceBalance
    // entries do not linger in the sparse matrix from a prior
    // computeJacobian at the same DOFs.
    prepareMatrixTag(_assembly, _var.number(), _lm_var_num);
    for (const auto i : make_range(_local_ke.m()))
      for (const auto j : make_range(_local_ke.n()))
        _local_ke(i, j) = 0.0;
    assignTaggedLocalMatrix();
    return;
  }

  // Precompute per-node normal projections (also decides which branch of
  // the NCP each node is on, for the transpose block).  Same
  // single-rank-with-ghosted-nodes assumption as computeResidual.
  const auto N = _node_ids.size();
  // `n_dot_dir[k]` = n_k . direction, used for (scalar_row, lambda_col):
  //   dR_s/dlambda_j = -w_j * (n_j . direction), which is a function of
  //   the USER-supplied load direction and can carry either sign.
  // `n_dot_axis[k]` = n_k . axis_hat = the k-th normal component along
  //   the positive Cartesian axis of the contactor's translation, used
  //   for (lambda_row, scalar_col): dR_lambda/ds = -c * (n_j . axis_hat).
  //   The contactor's translation is s * axis_hat regardless of the sign
  //   of `direction`, so this uses axis_hat and not direction -- mixing
  //   them up (an older bug) flips the Kls sign when the user picks a
  //   negative-axis load direction and Newton then diverges.
  std::vector<Real> n_dot_dir(N, 0.0);
  std::vector<Real> n_dot_axis(N, 0.0);
  std::vector<bool> gap_branch(N, false);
  // Compressed indexing again (see computeResidual): _lambda[j], _disp[j]
  // where j is the running count over accessible+semi-local nodes.
  std::size_t j = 0;
  for (const auto k : index_range(_node_ids))
  {
    if (!nodeIsAccessible(k))
      continue;
    const Node * node = _mesh.getMesh().node_ptr(_node_ids[k]);
    const auto q = _contactor.queryAt(deformedNodePoint(*node, j));
    n_dot_dir[k] = q.normal * _direction;
    n_dot_axis[k] = q.normal(_axis);
    gap_branch[k] = _c * q.gap < _lambda[j];
    ++j;
  }

  // (scalar_row, scalar_col): dR_s/ds = 0 in this formulation (F(t) does
  // not depend on s and the reaction depends on s only indirectly through
  // the geometric term dn/ds, which is a hessian order term we drop).
  // PETSc's sparsity still needs the entry, so assemble an explicit zero
  // -- plus an optional preconditioning shift (see `kss_stiffness` param
  // docstring).  The physically-motivated shift is
  //   Kss_precond = kss_stiffness * total_nodal_area * sign(direction . axis_hat)
  // The magnitude scales with total contact-patch area (a proxy for how
  // stiff the material's response is over the loaded region), and the
  // sign matches the true dR_s/ds sign: +1 when direction and axis_hat
  // agree (pushing s up increases reaction) and -1 when they oppose
  // (pushing s up decreases reaction).  Summing over ALL accessible
  // boundary nodes -- not just the currently-active-contact subset --
  // keeps the shift constant across Newton iterations so gap/lambda-branch
  // flipping in the min-NCP does not churn the effective diagonal.
  Real kss = 0.0;
  const Real kss_stiff = kssStiffness();
  if (kss_stiff > 0.0)
  {
    Real total_area = 0.0;
    for (const auto k : index_range(_node_ids))
    {
      if (!nodeIsAccessible(k))
        continue;
      const Node * node = _mesh.getMesh().node_ptr(_node_ids[k]);
      total_area += _nodal_area.nodalArea(node);
    }
    // _direction(_axis) is +/- 1 exactly (direction was normalized and
    // validated to be axis-aligned in the ctor).
    kss = kss_stiff * total_area * _direction(_axis);
  }
  prepareMatrixTag(_assembly, _var.number(), _var.number());
  for (const auto i : make_range(_local_ke.m()))
    for (const auto j : make_range(_local_ke.n()))
      _local_ke(i, j) = 0.0;
  _local_ke(0, 0) = kss;
  assignTaggedLocalMatrix();

  // (scalar_row, lambda_col): dR_s / dlambda_j = -w_j * (n_j . direction).
  prepareMatrixTag(_assembly, _var.number(), _lm_var_num);
  for (const auto i : make_range(_local_ke.m()))
    for (const auto j : make_range(_local_ke.n()))
      _local_ke(i, j) = 0.0;
  for (const auto k : index_range(_node_ids))
  {
    if (!nodeIsAccessible(k))
      continue;
    const Node * node = _mesh.getMesh().node_ptr(_node_ids[k]);
    const Real w = _nodal_area.nodalArea(node);
    _local_ke(0, k) = w * n_dot_dir[k];
  }
  assignTaggedLocalMatrix();

  // (lambda_row, scalar_col): the transpose block.  For each LM node on the
  // gap branch of min(lambda, c*g), R_lambda = c * g_LS(x_deformed - t(s)).
  // LevelSetContactor implements the translation as t(s) = s * axis_hat
  // (via disp_[axis]_scalar), where axis_hat is the POSITIVE Cartesian
  // unit vector on the relevant axis.  Hence
  //   dR_lambda / ds = c * grad(g_LS) . (-dt/ds)
  //                  = c * n . (-axis_hat)
  //                  = -c * (n . axis_hat).
  // The user-supplied `direction` is not used here -- it only enters the
  // (scalar_row, lambda_col) block above.
  // Lambda-branch nodes have R_lambda = lambda (independent of s).
  //
  // MOOSE's ScalarKernel dispatch (`addJacobianOffDiagScalar`) only fills
  // (scalar_row x field_col) blocks -- it never fills (field_row x scalar_col).
  // NodalKernel likewise has no scalar off-diagonal hook.  The idiomatic
  // workaround (MortarScalarBase pattern) is direct assembly via
  // TaggingInterface::addJacobian with explicit row/column DoF indices,
  // bypassing the tagged-block dispatch.
  const auto & lm_var = _sys.getVariable(_tid, _lm_var_num);
  const auto & scalar_dofs = _var.dofIndices();
  std::vector<dof_id_type> lm_dofs;
  std::vector<std::size_t> accessible_ks;
  for (const auto k : index_range(_node_ids))
    if (nodeIsAccessible(k))
      accessible_ks.push_back(k);
  lm_dofs.reserve(accessible_ks.size());
  for (const auto k : accessible_ks)
    lm_dofs.push_back(
        _mesh.getMesh().node_ref(_node_ids[k]).dof_number(_sys.number(), _lm_var_num, /*comp=*/0));
  DenseMatrix<Real> ke_transpose(accessible_ks.size(), 1);
  for (const auto i : index_range(accessible_ks))
    if (gap_branch[accessible_ks[i]])
      ke_transpose(i, 0) = -_c * n_dot_axis[accessible_ks[i]];
  if (!lm_dofs.empty())
    addJacobian(_assembly, ke_transpose, lm_dofs, scalar_dofs, lm_var.scalingFactor());
}

Real
RigidBodyLoadControl::kssStiffness() const
{
  if (_kss_stiffness_function)
    // Evaluate at the current simulation time.  Point argument is unused for
    // a time-only PiecewiseLinear/ParsedFunction; passing the origin is a
    // convention that matches how MOOSE evaluates other time-only functions
    // for scalar-valued preconditioning knobs.
    return _kss_stiffness_function->value(_fe_problem.time(), Point());
  return _kss_stiffness_constant;
}
