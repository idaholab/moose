# 2D test of advection-division with no upwinding

vx = 0.5
vy = 0.1
beta = '${vx} ${vy} 0'
k = 0.01

nx = 40
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = ${nx}
  ny = 40
[]

[Functions]
  [beta]
    type = ParsedVectorFunction
    value_x = ${vx}
    value_y = ${vy}
    value_z = 0
  []
  [box_start]
    type = ParsedFunction
    expression = 'if(x<0.2,if(y<0.7,if(y>0.3,1,0),0),0)'
  []
  [box_flux]
    type = ParsedFunction
    expression = 'if(x<0.2,if(y<0.7,if(y>0.3,0.2,0),0),0)'
  []
  [f_fn]
    type = ParsedFunction
    expression = 'if(x<0.5,if(y<0.7,if(y>0.3,-0.1,0),0),0)'
  []
[]

[Variables]
  [u]
  []
[]

[ICs]
  [u_blob]
    type = FunctionIC
    variable = u
    function = box_start
  []
[]

[Kernels]
  [udot]
    type = TimeDerivative
    variable = u
  []
  [advection]
    type = ConservativeAdvection
    variable = u
    velocity = ${beta}
  []
  [diffusion]
    type = CoefDiffusion
    variable = 'u'
    coef = ${k}
  []
  [force]
    type = BodyForce
    variable = u
    function = f_fn
  []
[]

[BCs]
  [inlet_flux]
    type = FunctionNeumannBC
    variable = 'u'
    boundary = 'left'
    function = box_flux
  []
  [outlet_avective_flux]
    type = ConservativeAdvectionBC
    variable = 'u'
    boundary = 'right'
    velocity_function = 'beta'
  []
  [outlet_diffusive_flux]
    type = DiffusionFluxBC
    variable = 'u'
    boundary = 'right'
  []
  # walls are 0 flux (achieved with no BC, this depends on the kernels used)
[]

[Postprocessors]
  [inlet_diffusive_flux]
    type = SideDiffusiveFluxIntegral
    variable = 'u'
    diffusivity = ${k}
    boundary = 'left'
  []
  [inlet_advective_flux]
    type = SideAdvectiveFluxIntegral
    advected_variable = 'u'
    component = 'normal'
    boundary = 'left'
    vel_x = ${vx}
    vel_y = ${vy}
  []
  [outlet_diffusive_flux]
    type = SideDiffusiveFluxIntegral
    variable = 'u'
    diffusivity = ${k}
    boundary = 'right'
  []
  [outlet_advective_flux]
    type = SideAdvectiveFluxIntegral
    advected_variable = 'u'
    component = 'normal'
    boundary = 'right'
    vel_x = ${vx}
    vel_y = ${vy}
  []
  [cfl]
    type = ParsedPostprocessor
    expression = '${vx} * dt / (1. / ${nx})'
    pp_names = 'dt'
  []
  [dt]
    type = TimestepSize
    outputs = 'none'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.1
  end_time = 1

  l_tol = 1e-14
  nl_abs_tol = 1e-14
  timestep_tolerance = 1e-10
[]

[Outputs]
  exodus = true
  csv = true
[]
