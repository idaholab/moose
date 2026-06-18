[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = -1.1
  ymin = -1.1
  xmax = 1.1
  ymax = 1.1
  elem_type = QUAD9
[]

[Variables]
  [u]
    family = LAGRANGE_VEC
    order = FIRST
  []
  [v]
  []
[]

[Kernels]
  [wave]
    type = KokkosVectorFEWave
    variable = u
    x_forcing_func = 'x_ffn'
    y_forcing_func = 'y_ffn'
  []
  [diff]
    type = KokkosDiffusion
    variable = v
  []
  [source]
    type = KokkosBodyForce
    variable = v
  []
  [advection]
    type = KokkosEFieldAdvection
    variable = v
    efield = u
    charge = 'positive'
    mobility = 100
  []
[]

[Functions]
  [x_ffn]
    type = KokkosParsedFunction
    expression = '(2*pi*pi + 1)*cos(pi*x)*sin(pi*y)'
  []
  [y_ffn]
    type = KokkosParsedFunction
    expression = '-(2*pi*pi + 1)*sin(pi*x)*cos(pi*y)'
  []
[]

[Preconditioning]
  [pre]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]
