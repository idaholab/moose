[Mesh]
  [disk]
    type = ConcentricCircleMeshGenerator
    has_outer_square = false
    preserve_volumes = false
    radii = '1 2'
    rings = '16 16'
    num_sectors = 16
  []
  [ring]
    type = BlockDeletionGenerator
    input = disk
    block = 1
    new_boundary = inner
  []
[]

[Variables]
  [T]
  []
[]

[Kernels]
  [diffusion]
    type = ADMatDiffusion
    variable = T
    diffusivity = k
    use_displaced_mesh = true
  []
  [src]
    type = ADBodyForce
    variable = T
    value = 1
    use_displaced_mesh = true
  []
[]

[BCs]
  [convection]
    type = ADMatNeumannBC
    boundary = inner
    variable = T
    boundary_material = convection
    value = 1
    use_displaced_mesh = true
  []
[]

[Materials]
  [conductivity]
    type = ADGenericConstantMaterial
    prop_names = 'k'
    prop_values = '1'
  []
  [convection]
    type = ADParsedMaterial
    expression = 'h * (100 - T)'
    coupled_variables = 'T'
    postprocessor_names = 'h'
    property_name = convection
    use_displaced_mesh = true
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [disp_x]
  []
  [disp_y]
  []
[]

[AuxKernels]
  [disp_x_aux]
    type = ParsedAux
    variable = disp_x
    expression = 'r:=sqrt(x * x + y * y);theta:=atan(y / x);
                  dr:=r * (thickness - 1) + inner_radius - thickness;
                  dr * cos(theta)'
    functor_names = 'inner_radius thickness'
    use_xyzt = true
    execute_on = 'timestep_begin'
    use_displaced_mesh = false
  []
  [disp_y_aux]
    type = ParsedAux
    variable = disp_y
    expression = 'r:=sqrt(x * x + y * y);theta:=atan(y / x);
                  dr:=r * (thickness - 1) + inner_radius - thickness;
                  dr * sin(theta)'
    functor_names = 'inner_radius thickness'
    use_xyzt = true
    execute_on = 'timestep_begin'
    use_displaced_mesh = false
  []
[]

[Postprocessors]
  [inner_radius]
    type = ConstantPostprocessor
    value = 6
    execute_on = 'timestep_begin'
    force_preaux = true
  []
  [thickness]
    type = ConstantPostprocessor
    value = 4
    execute_on = 'timestep_begin'
    force_preaux = true
  []

  [h]
    type = ParsedPostprocessor
    expression = '${fparse 10 / pi} / inner_radius^3'
    pp_names = 'inner_radius'
    execute_on = 'timestep_begin'
  []

  [Tmax]
    type = NodalExtremeValue
    variable = T
  []
  [volume]
    type = VolumePostprocessor
    use_displaced_mesh = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  line_search = none
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'
  nl_abs_tol = 1e-11
[]
