[GlobalParams]
  point_vector_metadata_values = '2.0 -1.0 3.0 2.0 2.0 -1.0;5.0 -2.0 3.0 1.0 -3.0 -2.0 4.0 -1.0 -2.5'
[]

[Mesh]
  [eg]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0
                       0 1 0'

    element_connectivity = '0 1 2 3'
    elem_type = "QUAD4"
  []
  [amdg]
    type = AddMetaDataGenerator
    input = eg
    real_scalar_metadata_names = 'rs_1 rs_2'
    real_scalar_metadata_values = '1.234 12.34'
    uint_scalar_metadata_names = 'uis_1'
    uint_scalar_metadata_values = '1234'
    int_scalar_metadata_names = 'is_1 is_2 is_3'
    int_scalar_metadata_values = '-1234 4321 -5678'
    dof_id_type_scalar_metadata_names = 'ds_1 ds_2'
    dof_id_type_scalar_metadata_values = '12 123'
    subdomain_id_type_scalar_metadata_names = 'ss_1 ss_2'
    subdomain_id_type_scalar_metadata_values = '21 321'
    boolean_scalar_metadata_names = 'bs_1 bs_2 bs_3'
    boolean_scalar_metadata_values = 'false true false'
    point_scalar_metadata_names = 'ps_1 ps_2'
    point_scalar_metadata_values = '1.0 -2.0 3.5
                                    -2.0 3.0 -1.5'

    real_vector_metadata_names = 'rv_1 rv_2'
    real_vector_metadata_values = '1.234 12.34 123.4;4.321 43.21'
    uint_vector_metadata_names = 'uiv_1'
    uint_vector_metadata_values = '1234 567 89'
    int_vector_metadata_names = 'iv_1 iv_2 iv_3'
    int_vector_metadata_values = '-1234 4321;-567 89;98 76 54'
    dof_id_type_vector_metadata_names = 'dv_1 dv_2'
    dof_id_type_vector_metadata_values = '12 123; 45 678 9'
    subdomain_id_type_vector_metadata_names = 'sv_1 sv_2'
    subdomain_id_type_vector_metadata_values = '21 321; 9 876 54'
    point_vector_metadata_names = 'pv_1 pv_2'
  []
  [test_meta]
    type = TestMeshMetaData
    input = amdg
  []
[]
