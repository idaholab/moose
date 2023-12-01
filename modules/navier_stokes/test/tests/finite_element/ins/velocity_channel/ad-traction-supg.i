# This input file tests outflow boundary conditions for the incompressible NS equations.

[GlobalParams]
  integrate_p_by_parts = true
  viscous_form = traction
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 3.0
  ymin = 0
  ymax = 1.0
  nx = 30
  ny = 10
  elem_type = QUAD9
[]


[Variables]
  [vel]
    order = SECOND
    family = LAGRANGE_VEC
  []
  [p]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [mass]
    type = INSADMass
    variable = p
  []
  [momentum_convection]
    type = INSADMomentumAdvection
    variable = vel
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = vel
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = vel
    pressure = p
  []
  [momentum_supg]
    type = INSADMomentumSUPG
    variable = vel
    velocity = vel
  []
[]

[BCs]
  [wall]
    type = VectorFunctionDirichletBC
    variable = vel
    boundary = 'top bottom'
    function_x = 0
    function_y = 0
  []
  [inlet]
    type = VectorFunctionDirichletBC
    variable = vel
    boundary = 'left'
    function_x = inlet_func
    function_y = 0
  []
[]

[Materials]
  [const]
    type = ADGenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '1  1'
  []
  [ins_mat]
    type = INSADTauMaterial
    velocity = vel
    pressure = p
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    solve_type = NEWTON
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  line_search = none
  nl_rel_tol = 1e-12
[]

[Outputs]
  [out]
    type = Exodus
  []
[]

[Functions]
  [inlet_func]
    type = ParsedFunction
    expression = '-4 * (y - 0.5)^2 + 1'
  []
[]
