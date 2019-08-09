[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  pore_shape_model = spherical
[]

[MeshGenerators]
  [./msh]
    type = CartesianMeshGenerator
    dim = 3
    dx = 0.01
    dy = 0.01
    dz = 0.01
    iz = 1
    ix = 1
    iy = 1
  [../]

  [./extra_nodeset]
    type = ExtraNodesetGenerator
    input = msh
    new_boundary = 'origin'
    coord = '0 0 0'
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Modules/TensorMechanics/Master/All]
  strain = FINITE
  add_variables = true
  generate_output = 'strain_xx strain_yy strain_xy hydrostatic_stress vonmises_stress'
[]

[Functions]
  [./Q_gtn]
    type = ParsedFunction
    vars = 'avg_vonmises gtn_gauge_stress'
    vals = 'avg_vonmises gtn_gauge_stress'
    value = 'avg_vonmises/gtn_gauge_stress'
  [../]
  [./M_gtn]
    type = ParsedFunction
    vars = 'avg_hydro gtn_gauge_stress'
    vals = 'avg_hydro gtn_gauge_stress'
    value = 'abs(avg_hydro) / gtn_gauge_stress'
  [../]
  [./Q_ten]
    type = ParsedFunction
    vars = 'avg_vonmises ten_gauge_stress'
    vals = 'avg_vonmises ten_gauge_stress'
    value = 'avg_vonmises/ten_gauge_stress'
  [../]
  [./M_ten]
    type = ParsedFunction
    vars = 'avg_hydro ten_gauge_stress'
    vals = 'avg_hydro ten_gauge_stress'
    value = 'abs(avg_hydro) / ten_gauge_stress'
  [../]
  [./Q_five]
    type = ParsedFunction
    vars = 'avg_vonmises five_gauge_stress'
    vals = 'avg_vonmises five_gauge_stress'
    value = 'avg_vonmises/five_gauge_stress'
  [../]
  [./M_five]
    type = ParsedFunction
    vars = 'avg_hydro five_gauge_stress'
    vals = 'avg_hydro five_gauge_stress'
    value = 'abs(avg_hydro) / five_gauge_stress'
  [../]
  [./Q_three]
    type = ParsedFunction
    vars = 'avg_vonmises three_gauge_stress'
    vals = 'avg_vonmises three_gauge_stress'
    value = 'avg_vonmises / three_gauge_stress'
  [../]
  [./M_three]
    type = ParsedFunction
    vars = 'avg_hydro three_gauge_stress'
    vals = 'avg_hydro three_gauge_stress'
    value = 'abs(avg_hydro) / three_gauge_stress'
  [../]
  [./Q_two]
    type = ParsedFunction
    vars = 'avg_vonmises two_gauge_stress'
    vals = 'avg_vonmises two_gauge_stress'
    value = 'avg_vonmises/two_gauge_stress'
  [../]
  [./M_two]
    type = ParsedFunction
    vars = 'avg_hydro two_gauge_stress'
    vals = 'avg_hydro two_gauge_stress'
    value = 'abs(avg_hydro) / two_gauge_stress'
  [../]
  [./Q_onepointfive]
    type = ParsedFunction
    vars = 'avg_vonmises onepointfive_gauge_stress'
    vals = 'avg_vonmises onepointfive_gauge_stress'
    value = 'avg_vonmises / onepointfive_gauge_stress'
  [../]
  [./M_onepointfive]
    type = ParsedFunction
    vars = 'avg_hydro onepointfive_gauge_stress'
    vals = 'avg_hydro onepointfive_gauge_stress'
    value = 'abs(avg_hydro) / onepointfive_gauge_stress'
  [../]
  [./Q_one]
    type = ParsedFunction
    vars = 'avg_vonmises one_gauge_stress'
    vals = 'avg_vonmises one_gauge_stress'
    value = 'avg_vonmises / one_gauge_stress'
  [../]
  [./M_one]
    type = ParsedFunction
    vars = 'avg_hydro one_gauge_stress'
    vals = 'avg_hydro one_gauge_stress'
    value = 'abs(avg_hydro) / one_gauge_stress'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e10
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputeMultiplePorousInelasticStress
    inelastic_models = 'gtn lps_ten lps_five lps_three lps_two lps_onepointfive lps_one'
    initial_porosity = 1e-3
    outputs = all
  [../]

  [./gtn]
    type = ViscoplasticityStressUpdate
    coefficient = 0
    power = 1 # arbitrary
    viscoplasticity_model = GTN
    base_name = gtn
    outputs = all
    relative_tolerance = 1e-30
  [../]
  [./lps_ten]
    type = ViscoplasticityStressUpdate
    coefficient = 0
    power = 10
    base_name = ten
    outputs = all
    relative_tolerance = 1e-30
  [../]
  [./lps_five]
    type = ViscoplasticityStressUpdate
    coefficient = 0
    power = 5
    base_name = five
    outputs = all
    relative_tolerance = 1e-30
  [../]
  [./lps_three]
    type = ViscoplasticityStressUpdate
    coefficient = 0
    power = 3
    base_name = three
    outputs = all
    relative_tolerance = 1e-30
  [../]
  [./lps_two]
    type = ViscoplasticityStressUpdate
    coefficient = 0
    power = 2
    base_name = two
    outputs = all
    relative_tolerance = 1e-30
  [../]
  [./lps_onepointfive]
    type = ViscoplasticityStressUpdate
    coefficient = 0
    power = 1.5
    base_name = onepointfive
    outputs = all
    relative_tolerance = 1e-30
    # constant_on = ELEMENT
  [../]
  [./lps_one]
    type = ViscoplasticityStressUpdate
    coefficient = 0
    power = 1
    base_name = one
    outputs = all
    relative_tolerance = 1e-30
    # constant_on = ELEMENT
  [../]

  [./const_stress]
    type = ComputeExtraStressConstant
    extra_stress_tensor = '1 1 1 1 1 1 1 1 1'
  [../]
[]

[BCs]
  [./no_disp_x]
    type = PresetBC
    variable = disp_x
    boundary = left
    value = 0.0
  [../]
  [./no_disp_y]
    type = PresetBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  [../]
  [./no_disp_z]
    type = PresetBC
    variable = disp_z
    boundary = back
    value = 0.0
  [../]
  [./Pressure]
    [./asdf]
      boundary = 'top right front'
      function = '10^(t/4.5)'
    [../]
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = FD
  num_steps = 10
  nl_abs_tol = 5e-10
[]

[Postprocessors]
  [./avg_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
  [../]
  [./avg_vonmises]
    type = ElementAverageValue
    variable = vonmises_stress
  [../]
  [./gtn_gauge_stress]
    type = ElementAverageValue
    variable = gtn_gauge_stress
    outputs = none
  [../]
  [./0Q_gtn]
    type = FunctionValuePostprocessor
    function = Q_gtn
  [../]
  [./0M_gtn]
    type = FunctionValuePostprocessor
    function = M_gtn
  [../]
  [./ten_gauge_stress]
    type = ElementAverageValue
    variable = ten_gauge_stress
    outputs = none
  [../]
  [./1Q_ten]
    type = FunctionValuePostprocessor
    function = Q_ten
  [../]
  [./1M_ten]
    type = FunctionValuePostprocessor
    function = M_ten
  [../]
  [./five_gauge_stress]
    type = ElementAverageValue
    variable = five_gauge_stress
    outputs = none
  [../]
  [./2Q_five]
    type = FunctionValuePostprocessor
    function = Q_five
  [../]
  [./2M_five]
    type = FunctionValuePostprocessor
    function = M_five
  [../]
  [./three_gauge_stress]
    type = ElementAverageValue
    variable = three_gauge_stress
    outputs = none
  [../]
  [./3Q_three]
    type = FunctionValuePostprocessor
    function = Q_three
  [../]
  [./3M_three]
    type = FunctionValuePostprocessor
    function = M_three
  [../]
  [./two_gauge_stress]
    type = ElementAverageValue
    variable = two_gauge_stress
    outputs = none
  [../]
  [./4Q_two]
    type = FunctionValuePostprocessor
    function = Q_two
  [../]
  [./4M_two]
    type = FunctionValuePostprocessor
    function = M_two
  [../]
  [./onepointfive_gauge_stress]
    type = ElementAverageValue
    variable = onepointfive_gauge_stress
    outputs = none
  [../]
  [./5Q_onepointfive]
    type = FunctionValuePostprocessor
    function = Q_onepointfive
  [../]
  [./5M_onepointfive]
    type = FunctionValuePostprocessor
    function = M_onepointfive
  [../]
  [./one_gauge_stress]
    type = ElementAverageValue
    variable = one_gauge_stress
    outputs = none
  [../]
  [./6Q_one]
    type = FunctionValuePostprocessor
    function = Q_one
  [../]
  [./6M_one]
    type = FunctionValuePostprocessor
    function = M_one
  [../]
[]

[Outputs]
  csv = true
  file_base = exact_spherical_out
[]
