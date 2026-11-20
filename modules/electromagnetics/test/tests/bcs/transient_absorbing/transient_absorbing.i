# Transient absorbing boundary condition on the real and imaginary parts of a vector
# field, used to check its Jacobian

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    elem_type = QUAD9
  []
[]

[Variables]
  [E_real]
    family = NEDELEC_ONE
    order = FIRST
  []
  [E_imag]
    family = NEDELEC_ONE
    order = FIRST
  []
[]

[Kernels]
  [curlcurl_real]
    type = CurlCurlField
    variable = E_real
  []
  [wave_real]
    type = VectorSecondTimeDerivative
    variable = E_real
  []
  [source_real]
    type = VectorBodyForce
    variable = E_real
    function_x = 0
    function_y = 'sin(pi*x)*sin(t)'
  []
  [curlcurl_imag]
    type = CurlCurlField
    variable = E_imag
  []
  [wave_imag]
    type = VectorSecondTimeDerivative
    variable = E_imag
  []
  [source_imag]
    type = VectorBodyForce
    variable = E_imag
    function_x = 0
    function_y = 'sin(pi*x)*(1-cos(t))'
  []
[]

[BCs]
  [absorb_real]
    type = VectorTransientAbsorbingBC
    variable = E_real
    coupled_field = E_imag
    component = real
    admittance = 795774.7154594767 # 1/mu_0
    boundary = 'left right top bottom'
  []
  [absorb_imag]
    type = VectorTransientAbsorbingBC
    variable = E_imag
    coupled_field = E_real
    component = imaginary
    admittance = 795774.7154594767 # 1/mu_0
    boundary = 'left right top bottom'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-14
  dt = 1
  num_steps = 3
  [TimeIntegrator]
    type = NewmarkBeta
  []
[]
