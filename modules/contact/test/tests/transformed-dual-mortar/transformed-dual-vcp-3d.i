# 3D uniform-compression transformed-dual mortar patch, preconditioned with the
# VariableCondensationPreconditioner (VCP). Shared setup (mesh, physics, contact) is in
# transformed-dual-3d-base.i; this input adds the VCP preconditioner and executioner. The
# spec drives the HEX20 (QUAD8 face) mesh here and the TET10 (TRI6 face) mesh via cli_args.
#
# The transform makes the coupling matrix D non-diagonal, so the exact-inverse path
# (is_lm_coupling_diagonal = false) is required; a companion RunApp case flips it to true to
# confirm the guardrail warning fires.

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
