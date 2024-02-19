[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[ProjectedStatefulMaterialStorage]
  [all]
    projected_props = 'test_real test_realvectorvalue test_ranktwotensor test_rankfourtensor'
    family = MONOMIAL
    order = FIRST
  []
[]

[Materials]
  [test]
    type = ProjectedStatefulPropertiesTestMaterial
  []
[]

[Postprocessors]
  [average_diff]
    type = ElementAverageMaterialProperty
    mat_prop = diff_norm
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
[]

[Debug]
  show_material_props = true
[]

[Outputs]
  csv = true
  # in initial the freshly set up old state of the material properties differes
  # from the interpolated state as the MAT->AUX->MAT dependency cannot be
  # resolved (as both the computation of the current state as well as the
  # testing are done in the same material object)
  execute_on = 'TIMESTEP_END'
[]
