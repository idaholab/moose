[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
[]

[Variables]
  [u]
    order = FIRST
    components = 2
    family = L2_LAGRANGE
  []
  [v]
    order = FIRST
    components = 2
    family = L2_LAGRANGE
  []
[]

[Kernels]
  [u_coupled_time_derivative]
    type = ArrayCoupledTimeDerivative
    variable = u
    v = v
  []
  [u_time_derivative]
    type = ArrayTimeDerivative
    variable = u
  []
  [u_diffusion]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = u_dc
  []
  [v_time_derivative]
    type = ArrayTimeDerivative
    variable = v
  []
  [v_diffusion]
    type = ArrayDiffusion
    variable = v
    diffusion_coefficient = v_dc
  []
[]

[ICs]
  [u]
    type = ArrayFunctionIC
    variable = u
    function = '2*(x+1) 3*(x+1)'
  []
  [v]
    type = ArrayFunctionIC
    variable = v
    function = '0.1*(x+1) 0.2*(x+1)'
  []
[]

[Materials]
  [u_dc]
    type = GenericConstantArray
    prop_name = u_dc
    prop_value = '1 1'
  []
  [v_dc]
    type = GenericConstantArray
    prop_name = v_dc
    prop_value = '2 2'
  []
[]


[AuxVariables]
  [u_grad_dot_x]
    order = FIRST
    family = L2_LAGRANGE
    components = 2
  []
[]

[AuxKernels]
  [u_grad_dot_x]
    type = CoupledArrayGradDotAux
    variable = u_grad_dot_x
    v = u
    grad_component = x
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
  dt = 0.1
  dtmin = 0.1
  num_steps = 3
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
