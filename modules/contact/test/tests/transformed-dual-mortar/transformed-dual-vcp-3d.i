# 3D uniform-compression transformed-dual mortar patch, preconditioned with the
# VariableCondensationPreconditioner (VCP). Same node-conforming HEX20 (QUAD8
# secondary face) patch as transformed-dual-patch-3d.i (shared setup in
# transformed-dual-3d-base.i), re-solved with VCP instead of the SMP / direct solve
# to exercise the VCP condensation path together with the transformed dual basis.
# The transform makes the coupling matrix D non-diagonal, so the exact-inverse path
# (is_lm_coupling_diagonal = false) must be used; the converged solution -- and
# therefore the spatially uniform contact pressure -- matches the direct solve. A
# companion RunApp case in the spec flips is_lm_coupling_diagonal = true to confirm
# the guardrail warning fires.

!include transformed-dual-3d-base.i

[Preconditioning]
  [vcp]
    type = VCP
    full = true
    lm_variable = 'normal_lm'
    primary_variable = 'disp_x'
    preconditioner = 'AMG'
    # The transformed dual basis makes the coupling matrix D non-diagonal, so
    # invert it exactly rather than with the diagonal approximation.
    is_lm_coupling_diagonal = false
    adaptive_condensation = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options = '-snes_converged_reason -ksp_converged_reason'

  line_search = 'none'

  dt = 0.5
  dtmin = 0.1
  end_time = 1.0

  l_max_its = 50

  nl_max_its = 20
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  snesmf_reuse_base = false
[]
