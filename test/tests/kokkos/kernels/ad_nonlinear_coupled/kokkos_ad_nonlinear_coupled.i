# Pointwise AD Jacobian of a kernel depending nonlinearly on the values and gradients of its own
# variable and a coupled nonlinear variable, with first- and second-order variables so the
# off-diagonal blocks contract shape functions of different FE types.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
  elem_type = QUAD9
[]

[Variables]
  [u]
    [InitialCondition]
      type = FunctionIC
      function = 'x * x + 0.5 * y + 0.3'
    []
  []
  [v]
    order = SECOND
    [InitialCondition]
      type = FunctionIC
      function = 'x * y + 0.2 * y * y + 0.1'
    []
  []
[]

[Kernels]
  [u_nonlinear]
    type = KokkosADNonlinearCoupledTest
    variable = u
    v = v
  []
  [v_diffusion]
    type = KokkosADDiffusion
    variable = v
  []
[]

[BCs]
  [u_left]
    type = KokkosADDirichletBC
    variable = u
    boundary = left
    value = 0.3
  []
  [v_right]
    type = KokkosADDirichletBC
    variable = v
    boundary = right
    value = 0.1
  []
  [u_flux]
    type = KokkosADCoupledVarNeumannBC
    variable = u
    boundary = top
    v = v
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]
