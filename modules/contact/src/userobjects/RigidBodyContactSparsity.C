//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RigidBodyContactSparsity.h"

#include "MooseMesh.h"
#include "MooseVariableFE.h"
#include "NonlinearSystemBase.h"
#include "SystemBase.h"

#include "libmesh/dof_map.h"
#include "libmesh/elem.h"
#include "libmesh/mesh_base.h"
#include "libmesh/boundary_info.h"

registerMooseObject("ContactApp", RigidBodyContactSparsity);

InputParameters
RigidBodyContactSparsity::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription("Preallocates the cross-node (LM, disp) Jacobian coupling that "
                             "RigidBodyNormalMechanicalContact writes on the contact sideset's "
                             "lower-d elements, but MOOSE's default sparsity computation misses.");
  params.addRequiredParam<VariableName>("lm_variable",
                                        "The Lagrange multiplier field variable on the contact "
                                        "lower-d block (same as the NCP kernel's `variable`).");
  params.addRequiredParam<std::vector<VariableName>>(
      "displacements", "Displacement variables in order (x, y[, z]).");
  params.addRequiredParam<std::vector<BoundaryName>>(
      "boundary",
      "Contact sideset id(s) -- same as the RigidBodyNodalNCPKernel/"
      "RigidBodyLoadControl `boundary`.  Used to mark boundary elements as "
      "semi-local on every rank so the scalar-owning rank of "
      "RigidBodyLoadControl can read `_lambda[k]` and `_disp[k]` for every "
      "node on the contact patch in parallel.");
  // Parallel: register GhostEverything so every rank sees every element
  // (owned + ghosted).  Two reasons:
  //   1. RigidBodyLoadControl is a NodalScalarKernel -- MOOSE only calls its
  //      residual/Jacobian on the rank that owns the scalar DoF (the last
  //      MPI rank), so that rank must have algebraic access to every LM
  //      DoF on the contact sideset.  GhostBoundary only ghosts higher-d
  //      elements incident to the boundary; the LM DoFs live on the
  //      lower-d subdomain and would still be missing.
  //   2. Our own augment_sparsity_pattern (below) iterates the lower-d
  //      block on each rank and adds LM<->disp coupling pairs; that
  //      iteration must see every lower-d element whose sparsity
  //      contribution touches a locally-owned row.
  // Rigid-body contact meshes are typically small (a few hundred lower-d
  // faces even in 3D), so full ghosting is cheap and bulletproof.
  params.addRelationshipManager("GhostEverything",
                                Moose::RelationshipManagerType::GEOMETRIC |
                                    Moose::RelationshipManagerType::ALGEBRAIC);
  // Attach in the constructor, which runs before es().init() where the
  // sparsity pattern is computed.  execute_on defaults to NONE (nothing to
  // do at runtime).
  params.set<ExecFlagEnum>("execute_on") = EXEC_NONE;
  return params;
}

RigidBodyContactSparsity::RigidBodyContactSparsity(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _lm_var_num(_fe_problem.getVariable(0, getParam<VariableName>("lm_variable")).number()),
    _ndisp(getParam<std::vector<VariableName>>("displacements").size()),
    _disp_var_num(_ndisp)
{
  const auto & disp_names = getParam<std::vector<VariableName>>("displacements");
  for (const auto k : make_range(_ndisp))
    _disp_var_num[k] = _fe_problem.getVariable(0, disp_names[k]).number();

  // Register via SystemBase's callback list rather than DofMap's object slot.
  // MOOSE already installs its own `extraSparsity` function on the DofMap; if
  // we ALSO attach to the object slot libMesh warns ("both a function AND
  // object..."), even though both actually fire.  Routing through the system's
  // callback list keeps everything in MOOSE's single function callback, no
  // warning, and gives us access to any future MOOSE-side augmentation
  // additions for free.
  auto & nl = _fe_problem.getNonlinearSystemBase(/*sys_num=*/0);
  nl.addExtraSparsityCallback([this](libMesh::SparsityPattern::Graph & sparsity,
                                     std::vector<libMesh::dof_id_type> & n_nz,
                                     std::vector<libMesh::dof_id_type> & n_oz)
                              { applyExtraSparsity(sparsity, n_nz, n_oz); });
}

void
RigidBodyContactSparsity::initialSetup()
{
  // Force every rank to treat all elements on the contact boundary as
  // ghosted (whether it owns them or not).  MooseMesh::isSemiLocal(node)
  // returns true only for nodes on active_local_elements + explicitly
  // ghosted elements -- GhostEverything's algebraic ghost is not enough to
  // populate that list.  Without this step, on a rank that owns no
  // boundary-adjacent element (e.g. the scalar-owning rank in
  // load-controlled contact) `NodalScalarKernel::reinit` skips every
  // boundary node and `_lambda[k]` / `_disp[k]` are empty vectors,
  // segfaulting on access.
  auto & mesh = _fe_problem.mesh().getMesh();
  const auto & binfo = mesh.get_boundary_info();
  const auto boundary_ids =
      _fe_problem.mesh().getBoundaryIDs(getParam<std::vector<BoundaryName>>("boundary"));
  const std::set<BoundaryID> bset(boundary_ids.begin(), boundary_ids.end());
  for (const auto & t : binfo.build_side_list())
  {
    const auto elem_id = std::get<0>(t);
    const auto bc_id = std::get<2>(t);
    if (bset.count(bc_id))
      _fe_problem.addGhostedElem(elem_id);
  }
}

void
RigidBodyContactSparsity::applyExtraSparsity(libMesh::SparsityPattern::Graph & sparsity,
                                             std::vector<libMesh::dof_id_type> & n_nz,
                                             std::vector<libMesh::dof_id_type> & n_oz)
{
  auto & nl = _fe_problem.getNonlinearSystemBase(/*sys_num=*/0);
  const auto & dof_map = nl.dofMap();
  auto & mesh = _fe_problem.mesh();
  const auto proc = processor_id();
  const auto first_dof_on_proc = dof_map.first_dof(proc);
  const auto end_dof_on_proc = dof_map.end_dof(proc);
  const auto n_dofs_on_proc = dof_map.n_local_dofs();
  const auto n_dofs_not_on_proc = dof_map.n_dofs() - dof_map.n_local_dofs();

  const auto & lm_var = nl.getVariable(0, _lm_var_num);
  const std::set<SubdomainID> & lm_blocks = lm_var.blockIDs();

  // For each lower-d block element visible to this rank (locally owned OR
  // ghosted), gather ALL DoFs from LM + every disp component (from BOTH
  // the lower-d element itself AND its higher-d parent).  Mark every pair
  // as coupled.  This closes the preallocation gap where MOOSE's
  // LowerDIntegratedBC assembles cross-node (LM,disp), (disp,LM), and
  // (disp_i_on_primary, disp_j_on_lower) blocks that libMesh's default
  // element-based sparsity misses.
  //
  // Parallel note: iterating `active_element_ptr_range` (not
  // `active_local_element_ptr_range`) is required in parallel.  The
  // AugmentSparsityPattern hook runs AFTER `Build::parallel_sync` on the
  // DofMap, so we can only write LOCAL rows of `sparsity` -- non-local
  // rows added here would be dropped.  For a lower-d element straddling a
  // partition boundary, the rank owning the LM DoF at one endpoint may
  // NOT own the lower-d element (or the disp DoF at the other endpoint).
  // Iterating including ghosts lets each rank independently see every
  // lower-d element touching its owned DoFs, and the per-row filter below
  // (r >= first_dof_on_proc, r < end_dof_on_proc) discards writes that
  // aren't local -- so the owning rank picks them up.  Reaches ghosted
  // elements because MOOSE registers `GhostLowerDElems` on any mesh with
  // a lower-d block.
  std::vector<libMesh::dof_id_type> all_dofs;
  std::vector<libMesh::dof_id_type> di;

  auto gather = [&](const libMesh::Elem * elem)
  {
    for (const auto k : make_range(_ndisp))
    {
      di.clear();
      dof_map.dof_indices(elem, di, _disp_var_num[k]);
      all_dofs.insert(all_dofs.end(), di.begin(), di.end());
    }
    di.clear();
    dof_map.dof_indices(elem, di, _lm_var_num);
    all_dofs.insert(all_dofs.end(), di.begin(), di.end());
  };

  for (const auto * elem : mesh.getMesh().active_element_ptr_range())
  {
    if (!lm_blocks.count(elem->subdomain_id()))
      continue;

    all_dofs.clear();
    gather(elem);
    // Also gather from the interior (higher-d) parent so we capture
    // (higher-d test) x (lower-d phi) blocks the LowerDIntegratedBC
    // creates via computeLowerDOffDiagJacobian(PrimaryLower, ...).
    if (const auto * parent = elem->interior_parent())
      gather(parent);

    std::sort(all_dofs.begin(), all_dofs.end());
    all_dofs.erase(std::unique(all_dofs.begin(), all_dofs.end()), all_dofs.end());
    if (all_dofs.empty())
      continue;

    for (const auto r : all_dofs)
    {
      if (r < first_dof_on_proc || r >= end_dof_on_proc)
        continue;
      const auto local = r - first_dof_on_proc;
      auto & row = sparsity[local];
      const auto old_size = row.size();
      for (const auto c : all_dofs)
      {
        if (!std::binary_search(row.begin(), row.begin() + old_size, c))
        {
          row.push_back(c);
          if (c < first_dof_on_proc || c >= end_dof_on_proc)
          {
            if (n_oz[local] < n_dofs_not_on_proc)
              n_oz[local]++;
          }
          else
          {
            if (n_nz[local] < n_dofs_on_proc)
              n_nz[local]++;
          }
        }
      }
      std::sort(row.begin() + old_size, row.end());
      std::inplace_merge(row.begin(), row.begin() + old_size, row.end());
    }
  }
}
