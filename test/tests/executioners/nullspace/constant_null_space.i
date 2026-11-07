# Pure Neumann diffusion of u has the constant field as its nullspace. The source has a nonzero
# mean, so the right hand side is not in the range of the Jacobian. The transpose nullspace mode
# removes that component. The Dirichlet problem for w is nonsingular, so the modes must have zero
# entries on the degrees of freedom of w.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  # A nonuniform mesh makes the nodal and the integral means of u differ
  bias_x = 1.3
[]

[Problem]
  constant_null_space_variables = 'u'
  constant_transpose_null_space_variables = 'u'
[]

[Variables]
  [u]
  []
  [w]
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [source_u]
    type = BodyForce
    variable = u
    function = x
  []
  [diff_w]
    type = Diffusion
    variable = w
  []
[]

[BCs]
  [left_w]
    type = DirichletBC
    variable = w
    boundary = left
    value = 0
  []
  [right_w]
    type = DirichletBC
    variable = w
    boundary = right
    value = 1
  []
[]

[Postprocessors]
  [u_left]
    type = PointValue
    variable = u
    point = '0 0 0'
  []
  [u_right]
    type = PointValue
    variable = u
    point = '1 0 0'
  []
  [u_dof_sum]
    type = NodalSum
    variable = u
  []
  [w_middle]
    type = PointValue
    variable = w
    point = '0.5 0 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  # PETSc removes the nullspace from the Krylov iterates only with left preconditioning
  petsc_options_iname = '-snes_type -ksp_type -pc_type -ksp_pc_side -ksp_norm_type'
  petsc_options_value = 'ksponly gmres jacobi left preconditioned'
  l_tol = 1e-12
  l_max_its = 100
[]

[Outputs]
  csv = true
[]
