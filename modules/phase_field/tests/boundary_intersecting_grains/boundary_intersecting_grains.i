[Mesh]
  # ImageMesh ignores nx, xmin, xmax (and similarly for y and z) and
  # tries to read them from the image file...
  type = ImageMesh
  dim = 2

  # Be sure to choose a corresponding image name below!
  # file = image001_cropped3_closing_298.png         # full size, 157 Mb Exodus file!
  # file = eighth_image001_cropped3_closing_298.png  # 1/8
  file = sixteenth_image001_cropped3_closing_298.png # 1/16

  # Uncomment to maintain 1:1 ratio between number of pixels and mesh size.
  # scale_to_one = false

  # Uncomment to set cells_per_pixel to something other than the default value of 1.0.
  # Must be <= 1.
  # cells_per_pixel = .75

  # To crop an image to e.g. 1/8th size, install ImageMagick and run:
  #  convert image001_cropped3_closing_298.png -crop 230x198+100+100 eighth_image001_cropped3_closing_298.png
  # Note: Do not use 'sips' on OSX to crop!  It actually interpolates
  # the colors in the image instead of just cropping.
[]

[Variables]
  [./u]
    # The FeatureFloodCount object requires an *nodal* variable in order
    # to work (otherwise it segfaults).
  [../]
[]

[AuxVariables]
  [./grain_auxvar]
    #order = CONSTANT
    #family = MONOMIAL
  [../]
  [./pid]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./nodal_flood_aux]
    # We definitely need to execute this AuxKernel on TIMESTEP_END,
    # otherwise the grain_auxvar won't be shown in paraview...
    execute_on = 'TIMESTEP_END'
    # This auxkernel is initialized *before* the variable u is set
    # from FunctionIC, so it will always be zero initially...
    variable = grain_auxvar
    type = FeatureFloodCountAux
    bubble_object = flood_count_pp
  [../]
  [./pid]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'initial timestep_end'
  [../]
[]


[Functions]
  [./tif]
    # ImageFunction gets its file range parameters from ImageMesh,
    # when it is present.  This prevents duplicating information in
    # input files.
    type = ImageFunction
  [../]
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    function = tif
    variable = u
  [../]
[]

[Postprocessors]
  [./flood_count_pp]
    execute_on = 'timestep_end'
    type = FeatureFloodCount
    variable = u
    # For some reason I don't understand yet, the ImageFunction thing
    # returns either 0 or 62735 for the images we have... so a value
    # of 1.0 is definitely between those two values :-P
    threshold = 1.0

    # We define the "bubbles/grains" to be regions where u is greater-than the threshold.
    use_less_than_threshold_comparison = false

    # File to write bubble volume data to
    bubble_volume_file = bubble_volumes.csv

    # Explicitly turn on the boundary-intersecting volume calculation
    compute_boundary_intersecting_volume = true
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
  use_legacy_uo_initialization = true
[../]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
