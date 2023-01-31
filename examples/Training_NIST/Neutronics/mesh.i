# Dimensions in m
height_assembly_z = 0.3302
height_gap_z = 0.0502
area_channel = 195.492e-6
width_assembly_x = 0.084709
width_fuel_plate_x = 0.00127

# In full assembly
n_channels = 18
n_fuel = 19

# Assuming flat plates
width_channel_x = ${fparse (width_assembly_x - n_fuel * width_fuel_plate_x) / n_channels }
width_channel_y = 0.0761

# Fuel Plate parameters
width_fuel_x = 0.0002159
width_clad_x = ${fparse (width_fuel_plate_x - width_fuel_x) / 2}
width_clad_y = 0.003683
width_fuel_y = ${fparse width_channel_y - width_clad_y * 2}
height_clad_outer_z = 0.0254
height_clad_inner_z = 0.009525
height_fuel_z = 0.295275

# Other parameters
exit_region_height = 0.1  # m
entry_region_height = 0.1 # m

# Mesh discretization
nx_channel = 4
nx_clad = 2
nx_fuel = 2
ny_clad = 2
ny_fuel = 10
axial_thickness = 0.01 # m
nz_per_m = ${fparse 1 / axial_thickness}

## Description of the system
# 2D channel between two plates, extruded to 3D
# - not bent
# - some error on width due to missing height on both sides of bend
# - full length, covers 2 assemblies and the gap, plus entry and exit region
# - Z is height
# - X is short component of the channel, from plate to plate
# - Y is long component of the channel, tangeant to plates

[Mesh]
  [base]
    type = CartesianMeshGenerator
    dim = 2

    dx = '${fparse width_channel_x / 2} ${width_clad_x} ${width_fuel_x} ${width_clad_x} ${fparse width_channel_x / 2}'
    ix = '${fparse nx_channel} ${nx_clad} ${nx_fuel} ${nx_clad} ${fparse nx_channel}'

    dy = '${width_clad_y} ${width_fuel_y} ${width_clad_y}'
    iy = '${ny_clad} ${ny_fuel} ${ny_clad}'

    subdomain_id = '0 1 1 1 0
                    0 1 2 1 0
                    0 1 1 1 0'

  []

  [extrude]
    type = AdvancedExtruderGenerator
    input = base
    direction = '0 0 1'
    bottom_boundary = '4'
    top_boundary = '5'

    heights = '${entry_region_height}

               ${height_clad_outer_z}
               ${height_fuel_z}
               ${height_clad_inner_z}

               ${height_gap_z}

               ${height_clad_inner_z}
               ${height_fuel_z}
               ${height_clad_outer_z}

               ${exit_region_height}'
    num_layers = '${fparse ceil(entry_region_height * nz_per_m)}

                  ${fparse ceil(height_clad_outer_z * nz_per_m)}
                  ${fparse ceil(height_fuel_z * nz_per_m)}
                  ${fparse ceil(height_clad_inner_z * nz_per_m)}

                  ${fparse ceil(height_gap_z * nz_per_m)}

                  ${fparse ceil(height_clad_inner_z * nz_per_m)}
                  ${fparse ceil(height_fuel_z * nz_per_m)}
                  ${fparse ceil(height_clad_outer_z * nz_per_m)}

                  ${fparse ceil(exit_region_height * nz_per_m)}'
    subdomain_swaps = '1 0 2 0;

                       2 1;
                       ;
                       2 1;

                       1 0 2 0;

                       2 1;
                       ;
                       2 1;

                       1 0 2 0'
  []

  [name_blocks]
    type = RenameBlockGenerator
    input = extrude
    old_block_id = '0 1 2'
    new_block_name = 'water clad fuel'
  []

  [add_boundaries]
    type = SideSetsBetweenSubdomainsGenerator
    input = name_blocks
    primary_block = 'water'
    paired_block = 'clad'
    new_boundary = 'clad_wall'
  []

  [split_boundaries]
    type = BreakBoundaryOnSubdomainGenerator
    input = 'add_boundaries'
    boundaries = 'bottom top'
  []

  [rename_boundaries]
    type = RenameBoundaryGenerator
    input = split_boundaries
    old_boundary = 'bottom_to_water top_to_water 4 5'
    new_boundary = 'assembly_wall assembly_wall front back'
  []
[]
