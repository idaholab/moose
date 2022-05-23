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
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxVariables]
  [./grain_auxvar]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./centroids]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./nodal_flood_aux]
    variable = grain_auxvar
    type = FeatureFloodCountAux
    flood_counter = flood_count_pp
    execute_on = 'initial timestep_end'
  [../]

  [./centroids]
    type = FeatureFloodCountAux
    variable = centroids
    flood_counter = flood_count_pp
    field_display = CENTROID
    execute_on = 'initial timestep_end'
  [../]
[]

[Functions]
  [./tif]
    # ImageFunction gets its file range parameters from ImageMesh,
    # when it is present.  This prevents duplicating information in
    # input files.
    type = ImageFunction

    # In these sample images the features we want to analyze are RED (or close to pure red). The
    # background is BLUE so we can easily distinguish between the two by selecting only the red channel.
    component = 0
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
    type = FeatureFloodCount
    variable = u
    threshold = 1.0
    compute_var_to_feature_map = true
    execute_on = 'initial timestep_end'
  [../]
[]

[VectorPostprocessors]
  [./grain_volumes]
    type = FeatureVolumeVectorPostprocessor
    flood_counter = flood_count_pp
    execute_on = 'initial timestep_end'
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[../]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
