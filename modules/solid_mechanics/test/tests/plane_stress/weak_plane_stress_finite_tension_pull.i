[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  out_of_plane_strain = strain_zz
[]

[Problem]
  extra_tag_vectors = 'ref'
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 1
  []
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [strain_zz]
  []
[]

[AuxVariables]
  [react_x]
  []
[]

[Postprocessors]
  [react_x]
    type = NodalSum
    variable = 'react_x'
    boundary = 'right'
  []
  [stress_xx]
    type = ElementalVariableValue
    variable = 'stress_xx'
    elementid = 0
  []
  [strain_zz]
    type = ElementalVariableValue
    variable = 'strain_zz'
    elementid = 0
  []
[]

[Modules/TensorMechanics/Master]
  [plane_stress]
    strain = FINITE
    planar_formulation = WEAK_PLANE_STRESS
    extra_vector_tags = 'ref'
    generate_output = 'stress_xx stress_xy stress_yy stress_zz strain_xx strain_xy strain_yy'
  []
[]

[AuxKernels]
  [react_x]
    type = TagVectorAux
    vector_tag = 'ref'
    v = 'disp_x'
    variable = 'react_x'
  []
[]

[BCs]
  [leftx]
    type = DirichletBC
    boundary = left
    variable = disp_x
    value = 0.0
  []
  [bottomy]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
    value = 0.0
  []
  [rightx]
    type = FunctionDirichletBC
    boundary = right
    variable = disp_x
    function = 't'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient

  solve_type = PJFNK
  line_search = none

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-10

  # time control
  start_time = 0.0
  dt = 0.01
  dtmin = 0.01
  end_time = 0.2
[]

[Outputs]
  csv = true
[]
