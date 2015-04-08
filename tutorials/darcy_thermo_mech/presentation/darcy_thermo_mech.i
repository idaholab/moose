[presentation]
  style = inl
  title = ThermomechanicalDarcyFlow
  [./cover]
    type = 'INLCoverSet'
    title = 'MOOSE Tutorial:<br>Thermomechanical Darcy Flow'
    contents_title = 'Tutorial Contents'
  [../]
  [./overview_cover]
    title = 'MOOSE Overview'
    type = 'INLMergeCoverSet'
    contents = 'true'
    slide_sets = 'overview moose-papers'
  [../]
  [./overview]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTraining/Overview'
    non_ascii_warn = 'false'
    [./Slides]
      [./moose]
        auto_title = 'False'
        class = 'bottom'
      [../]
      [./application-arch]
        auto_title = 'False'
        class = 'bottom'
      [../]
    [../]
    [./Images]
      align = 'center'
      [./73]
        width = '700px'
      [../]
      [./74]
        width = '675px'
      [../]
      [./75]
        width = '425px'
      [../]
      [./76]
        width = '250px'
      [../]
      [./77]
        width = '250px'
      [../]
      [./78]
        width = '250px'
      [../]
      [./79]
        width = '250px'
      [../]
    [../]
  [../]
  [./moose-papers]
    type = 'INLDjangoWikiSet'
    wiki = 'MoosePublications'
    non_ascii_warn = 'false'
    [./Slides]
      [./publications]
        prefix = '# MOOSE Results'
      [../]
    [../]
  [../]
  [./problem]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Problem'
    title = 'Problem Statement'
    [./Images]
      width = '570px'
    [../]
  [../]
  [./fem_cover]
    title = 'Finite Elements: The MOOSE Way'
    type = 'INLMergeCoverSet'
    contents = 'true'
    contents_title = 'FEM Contents'
    slide_sets = 'fem_principles fem_shape fem_numerical'
  [../]
  [./fem_principles]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTraining/FEM/Principles'
    [./Images]
      [./89]
        width = '420px'
      [../]
    [../]
  [../]
  [./fem_shape]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTraining/FEM/ShapeFunctions'
    [./Images]
      [./93]
        width = '290px'
        show_caption = 'false'
      [../]
      [./94]
        width = '290px'
        show_caption = 'false'
      [../]
      [./95]
        width = '290px'
        show_caption = 'false'
      [../]
      [./96]
        width = '290px'
        show_caption = 'false'
      [../]
      [./90]
        width = '235px'
        div_center = 'false'
      [../]
      [./91]
        width = '235px'
        div_center = 'false'
      [../]
      [./92]
        width = '235px'
        div_center = 'false'
      [../]
    [../]
  [../]
  [./fem_numerical]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTraining/FEM/NumericalImplementation'
  [../]
  [./input_file]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTraining/InputFile'
    contents = 'false'
    title = 'MOOSE Input File'
  [../]
  [./mesh]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseSystems/Mesh'
    contents = 'false'
    title = 'Mesh Block'
  [../]
  [./modifiers]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseSystems/MeshModifiers'
    contents = 'false'
    title = 'Mesh Modifiers'
  [../]
  [./outputs]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseSystems/Outputs'
    contents = 'false'
    title = 'Outputs'
  [../]
  [./anatomy]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTraining/MooseObject'
    contents = 'false'
    title = 'Anatomy of a MOOSE Object'
  [../]
  [./kernels]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseSystems/Kernels'
    contents = 'true'
    title = 'Kernels'
    inactive = 'time-derivative kernels-9 kernels-10 example-2:-adding-a-custom-kernel look-at-example-2'
    [./Images]
      [./105]
        width = '400px'
        text-align = 'center'
      [../]
    [../]
  [../]
  [./coords]
    title = 'Axisymmetric Coordinates'
    contents = 'false'
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTraining/CoordinateSystems'
    [./Images]
      [./177]
        width = '280px'
        show_caption = 'false'
      [../]
    [../]
  [../]
  [./step_01]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Step01'
    title = 'Step 1: Geometry and Diffusion'
    [./Images]
    [../]
  [../]
  [./input_parameters]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTraining/InputParameters'
    contents = 'false'
    title = 'Input Parameters and MOOSE Types'
  [../]
  [./step_02]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Step02'
    title = 'Step 2: Pressure Kernel'
    [./Images]
    [../]
  [../]
  [./materials]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseSystems/Materials'
    contents = 'true'
    contents_level = '2'
    title = 'Materials'
    inactive = 'example-8 example-9'
    [./Images]
      width = '350px'
    [../]
  [../]
  [./step_03]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Step03'
    title = 'Step 3: Pressure Kernel with Material'
    [./Images]
    [../]
  [../]
  [./aux_system]
    type = 'INLMergeCoverSet'
    slide_sets = 'aux_kernels aux_variables'
    title = 'Auxilliary System'
    contents = 'true'
  [../]
  [./aux_kernels]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseSystems/AuxKernels'
    inactive = 'example-10'
  [../]
  [./aux_variables]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseSystems/AuxVariables'
  [../]
  [./coupling]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTraining/MultiphysicsCoupling'
    inactive = 'example-3'
    title = 'Multiphysics Coupling'
  [../]
  [./step_04]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Step04'
    title = 'Step 4: Velocity Auxiliary Variable'
    [./Images]
    [../]
  [../]
  [./modules]
    type = 'INLDjangoWikiSet'
    wiki = 'PhysicsModules'
    title = 'MOOSE Modules'
    [./Images]
      width = '300px'
    [../]
  [../]
  [./step_05]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Step05'
    title = 'Step 5: Heat Conduction'
    [./Images]
    [../]
  [../]
  [./executioners]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseSystems/Executioners'
    contents = 'true'
    contents_level = '2'
    title = 'Executioners'
    inactive = 'example-6 example-6s eigenvalue-executioner'
    [./Images]
    [../]
  [../]
  [./step_05b]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Step05/Step05_transient'
    title = 'Step 5b: Transient Heat Conduction'
    [./Images]
    [../]
  [../]
  [./bcs]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseSystems/BCs'
    contents = 'true'
    contents_level = '2'
    title = 'Boundary Conditions'
    inactive = 'example-4 example-4o periodic-example'
    [./Images]
    [../]
  [../]
  [./step_05c]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Step05/Step05_outflow'
    title = 'Step 5c: Outflow BC'
    [./Images]
    [../]
  [../]
  [./step_06]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Step06'
    title = 'Step 6: Coupling Darcy and Heat Conduction'
    [./Images]
    [../]
  [../]
  [./adaptivity]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTraining/Adaptivity'
    contents = 'true'
    contents_level = '2'
    title = 'Mesh Adaptivity'
    inactive = 'example-5'
    [./Images]
    [../]
  [../]
  [./step_07]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Step07'
    title = 'Step 7: Mesh Adaptivity'
    [./Images]
    [../]
  [../]
  [./postprocessors]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseSystems/Postprocessors'
    contents = 'false'
    contents_level = '2'
    title = 'Postprocessors'
    [./Images]
    [../]
  [../]
  [./step_08]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Step08'
    title = 'Step 8: Posstprocessing'
    [./Images]
    [../]
  [../]
  [./step_09]
    type = 'INLDjangoWikiSet'
    wiki = 'MooseTutorials/DarcyThermoMechanical/Step09'
    title = 'Step 9: Adding Solid Mechanics'
    [./Images]
    [../]
  [../]
[]
