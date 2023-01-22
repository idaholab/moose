[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 4
  ny = 4
  nz = 4
[]

# [Variables]
#   [u]
#   []
# []
#
# [Kernels]
#   [diff]
#     type = CoefDiffusion
#     variable = u
#     coef = 0.1
#   []
#   [dt]
#     type = TimeDerivative
#     variable = u
#   []
# []

[AuxVariables]
  [xvar]
    family = MONOMIAL
    order = FIRST
  []
  [yvar]
  []
  [zvar]
    family = MONOMIAL
    order = CONSTANT
  []
  [tvar]
  []
[]

[AuxKernels]
  [xvar]
    type = ParsedAux
    variable = xvar
    use_xyzt = true
    expression = 'x+1'
  []
  [yvar]
    type = ParsedAux
    variable = yvar
    use_xyzt = true
    expression = 'y+2'
  []
  [zvar]
    type = ParsedAux
    variable = zvar
    use_xyzt = true
    expression = 'z+3'
  []
  [tvar]
    type = ParsedAux
    variable = tvar
    use_xyzt = true
    expression = 't+0.1*(x+y+z)'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Outputs]
  exodus = true
[]
