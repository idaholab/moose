[Mesh]
 type = GeneratedMesh
 dim = 2
 xmin = 0
 xmax = 10
 ymin = 0
 ymax = 10
 elem_type = QUAD4
 nx = 8
 ny = 8

 uniform_refine = 0
[]

# the minimum eigenvalue of this problem is 2*(PI/a)^2;
# Its inverse is 0.5*(a/PI)^2 = 5.0660591821169. Here a is equal to 10.

[Variables]
  active = 'u'

  [./u]
    # second order is way better than first order
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff rhs'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./rhs]
    type = MaterialEigenKernel
    variable = u
    mat = varmat
  [../]
[]

[Materials]
  [./var_mat]
    type = VarCouplingMaterialEigen
    block = 0
    var = u
    material_prop_name = varmat
  [../]
[]

[BCs]
  active = 'homogeneous'

  [./homogeneous]
    type = DirichletBC
    variable = u
    preset = false
    boundary = '0 1 2 3'
    value = 0
  [../]
[]

[Executioner]
  type = NonlinearEigen

  bx_norm = 'unorm'
  normalization = 'unorm'
  normal_factor = 9.990012561844

  free_power_iterations = 2
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-50
  k0 = 1.0

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Postprocessors]
  active = 'unorm udiff'

  [./unorm]
    type = ElementIntegralVariablePostprocessor
    variable = u
    # execute on residual is important for nonlinear eigen solver!
    execute_on = linear
  [../]

  [./udiff]
    type = ElementL2Diff
    variable = u
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = ne_mat
  exodus = true
[]
