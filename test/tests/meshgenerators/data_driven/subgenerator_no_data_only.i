[Mesh]
  [data]
    type = AddMetaDataGenerator
    real_scalar_metadata_names = 'scale'
    real_scalar_metadata_values = '8'
  []
  [test]
    type = TestDataDrivenGenerator
    subgenerator_no_data_only_from_generator = data
    subgenerator_no_data_only_scale_metadata = scale
  []

  data_driven_generator = test
[]
