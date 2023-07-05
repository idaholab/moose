[Mesh]
    [fmg]
      type = FileMeshGenerator
      file = 'Blanket_OneRow.msh'
    []
[]
  
[Outputs]
    exodus = true
[]
  
[Preconditioning]
    [smp]
      type = SMP
      full = true
    []
[]
  
[Executioner]
    type = Steady
[]
  
[Variables]
    [tritium]
    []
[]
  
[Kernels]
    [diffusion]
      type = ADDiffusion
      variable = tritium
      #temperature = temperature
    []
    [pd_breeder]
      type = BodyForce
      variable = tritium
      value = 1.71168e-11
      block = 'Breeder'
    []
[]
  
[BCs]
    [FW_BC]
      type = NeumannBC
      variable = tritium
      boundary = 'Heated_Surface'
      value = 0
    []
  
    [bw_bc]
      type = DirichletBC
      variable = tritium
      boundary = 'Back_Wall'
      value = 1e-6
    []

    [CH1_BC]
        type = NeumannBC
        variable = tritium
        boundary = 'CH1'
        value = 1.71168e-11
    []

    [CH2_BC]
        type = NeumannBC
        variable = tritium
        boundary = 'CH2'
        value = 1.71168e-11
    []
[]
  
[AuxVariables]
    [temperature]
    []
[]
  