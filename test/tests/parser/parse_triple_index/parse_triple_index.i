[Problem]
  solve = false
[]

[GlobalParams]
  sampler_name_tri = 'sampler_name000 ; sampler_name010 sampler_name011 sampler_name012 ; sampler_name020 sampler_name021 | sampler_name100 sampler_name101 |  sampler_name200 sampler_name201 ; sampler_name210 ; sampler_name220 sampler_name221 sampler_name222'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[UserObjects]
  [triple_index]
    type = ReadTripleIndex

    real_tri = ' 1.1 ; 2.1 2.2 2.3 ; 3.1 3.2 | 11.1 11.2 | 21.1 21.2 ; 22.1; 23.1 23.2 23.3'
    real_tri_empty_sub = ' 1.1 ; 2.1 2.2 2.3 ; 3.1 3.2 || 21.1 21.2 ; 22.1; 23.1 23.2 23.3'
    real_tri_empty_subsub = ' 1.1 ; 2.1 2.2 2.3 ; 3.1 3.2 | 11.1 11.2 |; 22.1; 23.1 23.2 23.3'
    real_tri_empty_subs = '|| 21.1 21.2 ; 22.1; 23.1 23.2 23.3'
    real_tri_empty_subsubs = ';; 3.1 3.2 || 21.1 21.2 ; 22.1; 23.1 23.2 23.3'
    real_tri_all_empty = '|;|'
    uint_tri = ' 11 ; 21 22 23 ; 31 32 | 111 112 | 211 212 ; 221; 231 232 233'
    int_tri = ' 11 ; -21 -22 -23 ; 31 32 | -111 -112 | 211 212 ; -221; 231 232 233'
    long_tri = ' -11 ; 21 22 23 ; -31 -32 | 111 112 | -211 -212 ; 221; -231 -232 -233'
    subid_tri = ' 41 ; 51 52 53 ; 61 62 | 141 142 | 241 242 ; 251; 261 262 263'
    bid_tri = ' 46 ; 56 57 58 ; 66 67 | 146 147 | 246 247 ; 256; 266 267 268'

    str_tri = 'string000 ; string010 string011 string012 ; string020 string021 | string100 string101 |  string200 string201 ; string210 ; string220 string221 string222'
    file_tri = 'file000 ; file010 file011 file012 ; file020 file021 | file100 file101 |  file200 file201 ; file210 ; file220 file221 file222'
    file_no_tri = 'file_no000 ; file_no010 file_no011 file_no012 ; file_no020 file_no021 | file_no100 file_no101 |  file_no200 file_no201 ; file_no210 ; file_no220 file_no221 file_no222'
    mesh_file_tri = 'mesh_file000 ; mesh_file010 mesh_file011 mesh_file012 ; mesh_file020 mesh_file021 | mesh_file100 mesh_file101 |  mesh_file200 mesh_file201 ; mesh_file210 ; mesh_file220 mesh_file221 mesh_file222'
    subdomain_name_tri = 'subdomain_name000 ; subdomain_name010 subdomain_name011 subdomain_name012 ; subdomain_name020 subdomain_name021 | subdomain_name100 subdomain_name101 |  subdomain_name200 subdomain_name201 ; subdomain_name210 ; subdomain_name220 subdomain_name221 subdomain_name222'
    boundary_name_tri = 'boundary_name000 ; boundary_name010 boundary_name011 boundary_name012 ; boundary_name020 boundary_name021 | boundary_name100 boundary_name101 |  boundary_name200 boundary_name201 ; boundary_name210 ; boundary_name220 boundary_name221 boundary_name222'
    function_name_tri = 'function_name000 ; function_name010 function_name011 function_name012 ; function_name020 function_name021 | function_name100 function_name101 |  function_name200 function_name201 ; function_name210 ; function_name220 function_name221 function_name222'
    userobject_name_tri = 'userobject_name000 ; userobject_name010 userobject_name011 userobject_name012 ; userobject_name020 userobject_name021 | userobject_name100 userobject_name101 |  userobject_name200 userobject_name201 ; userobject_name210 ; userobject_name220 userobject_name221 userobject_name222'
    indicator_name_tri = 'indicator_name000 ; indicator_name010 indicator_name011 indicator_name012 ; indicator_name020 indicator_name021 | indicator_name100 indicator_name101 |  indicator_name200 indicator_name201 ; indicator_name210 ; indicator_name220 indicator_name221 indicator_name222'
    marker_name_tri = 'marker_name000 ; marker_name010 marker_name011 marker_name012 ; marker_name020 marker_name021 | marker_name100 marker_name101 |  marker_name200 marker_name201 ; marker_name210 ; marker_name220 marker_name221 marker_name222'
    multiapp_name_tri = 'multiapp_name000 ; multiapp_name010 multiapp_name011 multiapp_name012 ; multiapp_name020 multiapp_name021 | multiapp_name100 multiapp_name101 |  multiapp_name200 multiapp_name201 ; multiapp_name210 ; multiapp_name220 multiapp_name221 multiapp_name222'
    postprocessor_name_tri = 'postprocessor_name000 ; postprocessor_name010 postprocessor_name011 postprocessor_name012 ; postprocessor_name020 postprocessor_name021 | postprocessor_name100 postprocessor_name101 |  postprocessor_name200 postprocessor_name201 ; postprocessor_name210 ; postprocessor_name220 postprocessor_name221 postprocessor_name222'
    vector_postprocessor_name_tri = 'vector_postprocessor_name000 ; vector_postprocessor_name010 vector_postprocessor_name011 vector_postprocessor_name012 ; vector_postprocessor_name020 vector_postprocessor_name021 | vector_postprocessor_name100 vector_postprocessor_name101 |  vector_postprocessor_name200 vector_postprocessor_name201 ; vector_postprocessor_name210 ; vector_postprocessor_name220 vector_postprocessor_name221 vector_postprocessor_name222'
    output_name_tri = 'output_name000 ; output_name010 output_name011 output_name012 ; output_name020 output_name021 | output_name100 output_name101 |  output_name200 output_name201 ; output_name210 ; output_name220 output_name221 output_name222'
    material_property_name_tri = 'material_property_name000 ; material_property_name010 material_property_name011 material_property_name012 ; material_property_name020 material_property_name021 | material_property_name100 material_property_name101 |  material_property_name200 material_property_name201 ; material_property_name210 ; material_property_name220 material_property_name221 material_property_name222'
    material_name_tri = 'material_name000 ; material_name010 material_name011 material_name012 ; material_name020 material_name021 | material_name100 material_name101 |  material_name200 material_name201 ; material_name210 ; material_name220 material_name221 material_name222'
    moose_functor_name_tri = 'moose_functor_name000 ; moose_functor_name010 moose_functor_name011 moose_functor_name012 ; moose_functor_name020 moose_functor_name021 | moose_functor_name100 moose_functor_name101 |  moose_functor_name200 moose_functor_name201 ; moose_functor_name210 ; moose_functor_name220 moose_functor_name221 moose_functor_name222'
    distribution_name_tri = 'distribution_name000 ; distribution_name010 distribution_name011 distribution_name012 ; distribution_name020 distribution_name021 | distribution_name100 distribution_name101 |  distribution_name200 distribution_name201 ; distribution_name210 ; distribution_name220 distribution_name221 distribution_name222'
  []
[]

[Executioner]
  type = Steady
[]
