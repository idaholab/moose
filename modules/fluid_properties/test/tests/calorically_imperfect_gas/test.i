[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Functions]
  [f_fn]
    type = ParsedFunction
    expression = -4
  []
  [bc_fn]
    type = ParsedFunction
    expression = 'x*x+y*y'
  []

  [e_fn]
    type = PiecewiseLinear
    x = '100   280 300 350 400 450 500 550 600 700 800 900 1000 1200 1400 1600 1800 2000 2200 2400 2600 2800 3000 5000'
    y = '783.9 2742.3 2958.6 3489.2 4012.7 4533.3 5053.8 5574 6095.1 7140.2 8192.9 9256.3 10333.6 12543.9 14836.6 17216.3 19688.4 22273.7 25018.3 28042.3 31544.2 35818.1 41256.5 100756.5'
    scale_factor = 1e3
  []

  [mu_fn]
    type = PiecewiseLinear
    x = '100   280 300 350 400 450 500 550 600 700 800 900 1000 1200 1400 1600 1800 2000 2200 2400 2600 2800 3000 5000'
    y = '85.42 85.42 89.53 99.44 108.9 117.98 126.73 135.2 143.43 159.25 174.36 188.9 202.96 229.88 255.5 280.05 303.67 326.45 344.97 366.49 387.87 409.48 431.86 431.86'
    scale_factor = 1e-7
  []

  [k_fn]
    type = PiecewiseLinear
    x = '100 280 300 350 400 450 500 550 600 700 800 900 1000 1200 1400 1600 1800 2000 2200 2400 2600 2800 3000 5000'
    y = '186.82 186.82 194.11 212.69 231.55 250.38 268.95 287.19 305.11 340.24 374.92 409.66 444.75 511.13 583.42 656.44 733.32 826.53 961.15 1180.38 1546.31 2135.49 3028.08 3028.08'
    scale_factor = 1e-3
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [e]
    initial_condition = 4012.7e3
  []
  [v]
    initial_condition = 0.0007354064593540647
  []

  [p]
    family = MONOMIAL
    order = CONSTANT
  []
  [T]
    family = MONOMIAL
    order = CONSTANT
  []
  [cp]
    family = MONOMIAL
    order = CONSTANT
  []
  [cv]
    family = MONOMIAL
    order = CONSTANT
  []
  [c]
    family = MONOMIAL
    order = CONSTANT
  []
  [mu]
    family = MONOMIAL
    order = CONSTANT
  []
  [k]
    family = MONOMIAL
    order = CONSTANT
  []
  [g]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [p]
    type = MaterialRealAux
     variable = p
     property = pressure
  []
  [T]
    type = MaterialRealAux
     variable = T
     property = temperature
  []
  [cp]
    type = MaterialRealAux
     variable = cp
     property = cp
  []
  [cv]
    type = MaterialRealAux
     variable = cv
     property = cv
  []
  [c]
    type = MaterialRealAux
     variable = c
     property = c
  []
  [mu]
    type = MaterialRealAux
     variable = mu
     property = mu
  []
  [k]
    type = MaterialRealAux
     variable = k
     property = k
  []
  [g]
    type = MaterialRealAux
     variable = g
     property = g
  []
[]

[FluidProperties]
  [h2]
    type = CaloricallyImperfectGas
    molar_mass = 0.002
    e = e_fn
    k = k_fn
    mu = mu_fn
    min_temperature = 100
    max_temperature = 5000
  []
[]

[Materials]
  [fp_mat]
    type = FluidPropertiesMaterialVE
    e = e
    v = v
    fp = h2
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [ffn]
    type = BodyForce
    variable = u
    function = f_fn
  []
[]

[BCs]
  [all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'left right top bottom'
    function = bc_fn
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
