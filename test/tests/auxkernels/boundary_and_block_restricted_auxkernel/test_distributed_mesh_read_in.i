[Mesh]
    file = 2D_4elem_2subdomain.e
[]

  [Functions]
    [./fn_exact]
      type = ParsedFunction
      value = 'x*x+y*y'
    [../]

    [./ffn]
      type = ParsedFunction
      value = -4
    [../]
  []


  [Variables]
    [./u]
      family = LAGRANGE
      order = FIRST
    [../]
  []


  [Kernels]
    [./diff]
      type = Diffusion
      variable = u
    [../]

    [./ffn]
      type = BodyForce
      variable = u
      function = ffn
    [../]
  []

  [BCs]
    [./all]
      type = FunctionDirichletBC
      variable = u
      boundary = '0 1 2 3'
      function = fn_exact
    [../]
  []

  [Materials]
    [./stateful1]
      type = StatefulMaterial
      block = 0
      initial_diffusivity = 5
    [../]
    [./stateful2]
      type = StatefulMaterial
      block = 1
      initial_diffusivity = 2
    [../]
  []

  [AuxKernels]
    [./diffusivity_1]
      type = MaterialRealAux
      property = diffusivity
      variable = diffusivity_1
      block = 0
    []
    [./diffusivity_2]
      type = MaterialRealAux
      property = diffusivity
      variable = diffusivity_2
      block = 1
    []
  []

  [AuxVariables]
    [./diffusivity_1]
      family = MONOMIAL
      order = CONSTANT
    []
    [./diffusivity_2]
      family = MONOMIAL
      order = CONSTANT
    []
  []


  [Executioner]
    type = Steady
    solve_type = NEWTON
  []

  [Outputs]
    exodus = true
  []
