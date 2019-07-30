[Tests]
  issues = '#13496'
  design = 'ComputeBlockRotatedElasticityTensor.md'
  
  
  [./block_rotated_elasticity_tensor]
    type = Exodiff
    input = 'block_rotated_elasticity_tensor_test.i'
    exodiff = 'block_rotated_elasticity_tensor_test_out.e'
    allow_test_objects = true
    requirement = 'The ComputeBlockRotatedElasticityTensor class shall correctly compute the rotated elasticity tensor for every block after rotating those according to provideded euler angles .'
  [../]
[]
