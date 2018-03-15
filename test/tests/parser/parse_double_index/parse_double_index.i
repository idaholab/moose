[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[UserObjects]
  [./double_index]
    type = ReadDoubleIndex
    real_di = ' 1.1 ; 2.1 2.2 2.3 ; 3.1 3.2'
    uint_di = ' 11 ; 21 22 23 ;
                31 32'
    int_di = ' 11 ; -21 -22 -23 ;
                31 32'
    long_di = ' -11 ; 21 22 23 ; -31 -32'
    subid_di = '22 ; 32 33 34 ; 42 43'
    bid_di = '21 ; 31 32 33 ; 41 42'

    str_di = 'string00 ; string10 string11 string12 ; string20 string21 '
    file_di = 'file00; file10 file11 file12; file20 file21'
    file_no_di = 'file_no00; file_no10 file_no11 file_no12; file_no20 file_no21'
    mesh_file_di = 'mesh_file00; mesh_file10 mesh_file11 mesh_file12; mesh_file20 mesh_file21'
    subdomain_name_di = 'subdomain_name00; subdomain_name10 subdomain_name11 subdomain_name12; subdomain_name20 subdomain_name21'
    boundary_name_di = 'boundary_name00; boundary_name10 boundary_name11 boundary_name12; boundary_name20 boundary_name21'
    function_name_di = 'function_name00; function_name10 function_name11 function_name12; function_name20 function_name21'
    userobject_name_di = 'userobject_name00; userobject_name10 userobject_name11 userobject_name12; userobject_name20 userobject_name21'
    indicator_name_di = 'indicator_name00; indicator_name10 indicator_name11 indicator_name12; indicator_name20 indicator_name21'
    marker_name_di = 'marker_name00; marker_name10 marker_name11 marker_name12; marker_name20 marker_name21'
    multiapp_name_di = 'multiapp_name00; multiapp_name10 multiapp_name11 multiapp_name12; multiapp_name20 multiapp_name21'
    postprocessor_name_di = 'postprocessor_name00; postprocessor_name10 postprocessor_name11 postprocessor_name12; postprocessor_name20 postprocessor_name21'
    vector_postprocessor_name_di = 'vector_postprocessor_name00; vector_postprocessor_name10 vector_postprocessor_name11 vector_postprocessor_name12; vector_postprocessor_name20 vector_postprocessor_name21'
    output_name_di = 'output_name00; output_name10 output_name11 output_name12; output_name20 output_name21'
    material_property_name_di = 'material_property_name00; material_property_name10 material_property_name11 material_property_name12; material_property_name20 material_property_name21'
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'


  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  file_base = parse_double_index
[]
