# Advanced Burner Test Reactor - Heterogeneous 3D Core using Reactor Geometry Mesh Builder (RGMB)

# ==============================================================================
# Global Variables
# ==============================================================================
# These global variables control geometrical parameters of the heterogeneous mesh
fuel_pin_pitch = 0.908             # Pin pitch of fuel pin
fuel_clad_r_i = 0.348              # Inner radius of fuel pin cladding
fuel_clad_r_o = 0.40333            # Outer radius of fuel pin cladding
control_pin_pitch = 1.243          # Pin pitch of control pin
control_clad_r_i = 0.485           # Inner radius of control pin cladding
control_clad_r_o = 0.55904         # Outer radius of control pin cladding
control_duct_pitch_inner = 12.198  # Inner pitch of control assembly duct region
control_duct_pitch_outer = 12.798  # Outer pitch of control assembly duct region
duct_pitch_inner = 13.598          # Inner pitch of fuel assembly duct region
duct_pitch_outer = 14.198          # Outer pitch of fuel assembly duct region
assembly_pitch = 14.598            # Assembly pitch

# These global variables control the placement of axial levels in the extruded mesh
z_active_core_lower = 60
z_active_core_upper = 140
z_sodium_gp_upper = 160
z_gp_upper = 260

# These global variables control the size of each axial layer
dz_active_core_lower = '${fparse z_active_core_lower - 0}'
dz_active_core_upper = '${fparse z_active_core_upper - z_active_core_lower}'
dz_sodium_gp_upper = '${fparse z_sodium_gp_upper - z_active_core_upper}'
dz_gp_upper = '${fparse z_gp_upper - z_sodium_gp_upper}'

# These global variables control how many intervals are in each axial layer. Here, a
# maximum axial mesh size of 20cm is assumed for each axial subinterval
max_axial_mesh_size = 20
naxial_active_core_lower = '${fparse ceil(dz_active_core_lower / max_axial_mesh_size)}'
naxial_active_core_upper = '${fparse ceil(dz_active_core_upper / max_axial_mesh_size)}'
naxial_sodium_gp_upper = '${fparse ceil(dz_sodium_gp_upper / max_axial_mesh_size)}'
naxial_gp_upper = '${fparse ceil(dz_gp_upper / max_axial_mesh_size)}'

# These global variables assign a region ID to each region in the core
# RGMB uses these region IDs to assign the `region_id` extra element integer
mid_fuel_1 = 1           # Fuel region of fuel_pin_1
mid_fuel_2 = 2           # Fuel region of fuel_pin_2
mid_fuel_3 = 3           # Fuel region of fuel_pin_3
mid_b4c = 4              # B4C region of control pin
mid_ht9 = 5              # HT-9 steel region of cladding
mid_sodium = 6           # Sodium region of background
mid_rad_refl = 7         # Radial reflector region
mid_rad_shld = 8         # Radial shielding region
mid_lower_refl = 9       # Lower reflector region
mid_upper_na_plen = 10   # Sodium plenum region
mid_upper_gas_plen = 11  # Gas plenum region
mid_control_empty = 12   # Empty region of control assembly

[Mesh]

  #################################      # This parameter allows us to execute the file but stop at this block so we can see intermediate output.
  final_generator = het_core             # User: Change this to 'fuel_pin_1', 'fuel_assembly_1', or 'reflector_assembly', or 'het_core'
                                         # Note: If `extrude = true` is not specified in PinMeshGenerator or AssemblyMeshGenerator, a 2-D mesh is
                                         #       created. To visualize the intermediate 3-D mesh, `extrude = true` needs to be moved from CoreMeshGenerator to
                                         #       the relevant mesh block, as this parameter can be set to true only once in the input file. All subsequent
                                         #       lines after the mesh block of interest also need to be commented out.

  # step 1: fuel_pin_1 (pin mesh structure to stitch into fuel assemblies)
  # step 2: fuel_assembly_1 (assembly mesh structure to define fuel assemblies)
  # step 3: reflector_assembly (homogenized assembly mesh structure to stitch into core)
  # step 4: het_core (final heterogeneous ABTR core mesh structure)

  #################################

  ### Step 0. Define global parameters for Reactor Geometry Mesh Builder workflow
              # Note: This step does not produce any mesh
  [rmp]
    type = ReactorMeshParams
    dim = 3                               # Dimensionality of output mesh (2 or 3)
    geom = "Hex"                          # Geometry type (Hex or Square)
    assembly_pitch = ${assembly_pitch}    # # Size of assembly flat-to-flat pitch

    axial_regions = '${dz_active_core_lower}
                     ${dz_active_core_upper}
                     ${dz_sodium_gp_upper}
                     ${dz_gp_upper}'                    # Size of each axial zone
    axial_mesh_intervals = '${naxial_active_core_lower}
                            ${naxial_active_core_upper}
                            ${naxial_sodium_gp_upper}
                            ${naxial_gp_upper}'         # Number of subintervals per axial zone
    top_boundary_id = 201                               # Boundary ID assigned to top surface
    bottom_boundary_id = 202                            # Boundary ID assigned to bottom surface
    radial_boundary_id = 203                            # Boundary ID assigned to radial surface
    flexible_assembly_stitching = true                  # Set to true to stitch dissimilar assembly types together,
                                                        # i.e. homogeneous and heterogeneous assemblies
  []

  ### Step 1. Define pin mesh structures to stitch into fuel / control assemblies
  [fuel_pin_1]
    type = PinMeshGenerator
    reactor_params = rmp                              # Name of ReactorMeshParams object
    pin_type = 1                                      # Unique identifier for pin type
    pitch = ${fuel_pin_pitch}                         # Pin pitch
    num_sectors = 2                                   # Number of azimuthal sectors per hexagonal side
    quad_center_elements = false                      # Whether central mesh elements in inner ring should use
                                                      # quad elements (true) or tri elements (false)
    ring_radii = '${fuel_clad_r_i} ${fuel_clad_r_o}'  # Radii for each ring
    mesh_intervals = '1 1 1'                          # Number of radial intervals for each radial region
                                                      # (inner ring, outer ring, background)
    region_ids = '${mid_lower_refl}     ${mid_lower_refl}     ${mid_lower_refl};
                  ${mid_fuel_1}         ${mid_ht9}            ${mid_sodium};
                  ${mid_upper_na_plen}  ${mid_upper_na_plen}  ${mid_upper_na_plen};
                  ${mid_upper_gas_plen} ${mid_upper_gas_plen} ${mid_upper_gas_plen}' # Region IDs for each radial region (inner ring, outer ring, background),
                                                                                     # provided for each axial layer from bottom to top
  []
  [fuel_pin_2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = ${fuel_pin_pitch}
    num_sectors = 2
    quad_center_elements = false
    ring_radii = '${fuel_clad_r_i} ${fuel_clad_r_o}'
    mesh_intervals = '1 1 1'
    region_ids = '${mid_lower_refl}     ${mid_lower_refl}     ${mid_lower_refl};
                  ${mid_fuel_2}         ${mid_ht9}            ${mid_sodium};
                  ${mid_upper_na_plen}  ${mid_upper_na_plen}  ${mid_upper_na_plen};
                  ${mid_upper_gas_plen} ${mid_upper_gas_plen} ${mid_upper_gas_plen}'
  []
  [fuel_pin_3]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = ${fuel_pin_pitch}
    num_sectors = 2
    quad_center_elements = false
    ring_radii = '${fuel_clad_r_i} ${fuel_clad_r_o}'
    mesh_intervals = '1 1 1'
    region_ids = '${mid_lower_refl}     ${mid_lower_refl}     ${mid_lower_refl};
                  ${mid_fuel_3}         ${mid_ht9}            ${mid_sodium};
                  ${mid_upper_na_plen}  ${mid_upper_na_plen}  ${mid_upper_na_plen};
                  ${mid_upper_gas_plen} ${mid_upper_gas_plen} ${mid_upper_gas_plen}'
  []

  [control_pin]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 4
    pitch = ${control_pin_pitch}
    num_sectors = 2
    quad_center_elements = false
    ring_radii = '${control_clad_r_i} ${control_clad_r_o}'
    mesh_intervals = '1 1 1' # Fuel, cladding, background
    region_ids = '${mid_control_empty} ${mid_control_empty} ${mid_control_empty};
                  ${mid_control_empty} ${mid_control_empty} ${mid_control_empty};
                  ${mid_b4c}            ${mid_ht9}            ${mid_sodium};
                  ${mid_b4c}            ${mid_ht9}            ${mid_sodium}'
  []

  ### Step 2. Define assembly mesh structures for fuel and control assemblies
  [fuel_assembly_1]
    type = AssemblyMeshGenerator
    assembly_type = 1                              # Unique identifier for pin type

    background_intervals = 1                       # Number of radial intervals in background region
    background_region_id = '${mid_lower_refl}
                            ${mid_sodium}
                            ${mid_upper_na_plen}
                            ${mid_upper_gas_plen}'    # Region ID corresponding to background region,
                                                      # defined for each axial layer from bottom to top
    duct_halfpitch = '${fparse duct_pitch_inner / 2}
                      ${fparse duct_pitch_outer / 2}' # Halfpitches for assembly inner and outer duct regions
    duct_intervals = '1 1'                            # Number of radial intervals for each assembly duct region
    duct_region_ids = ' ${mid_lower_refl}     ${mid_lower_refl};
                        ${mid_ht9}            ${mid_sodium};
                        ${mid_upper_na_plen}  ${mid_upper_na_plen};
                        ${mid_upper_gas_plen} ${mid_upper_gas_plen}' # Region IDs corresponding to inner and outer duct regions,
                                                                     # defined for each axial layer from bottom to top

    inputs = 'fuel_pin_1'                    # Name of contituent pin mesh structures
    pattern = '0 0 0 0 0 0 0 0 0;
              0 0 0 0 0 0 0 0 0 0;
             0 0 0 0 0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0 0 0 0 0;
           0 0 0 0 0 0 0 0 0 0 0 0 0;
          0 0 0 0 0 0 0 0 0 0 0 0 0 0;
         0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
       0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
         0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
          0 0 0 0 0 0 0 0 0 0 0 0 0 0;
           0 0 0 0 0 0 0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0 0 0 0 0;
             0 0 0 0 0 0 0 0 0 0 0;
              0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0'            # Lattice pattern of constituent pins
  []
  [fuel_assembly_2]
    type = AssemblyMeshGenerator
    assembly_type = 2
    background_intervals = 1
    background_region_id = '${mid_lower_refl} ${mid_sodium} ${mid_upper_na_plen} ${mid_upper_gas_plen}'
    duct_halfpitch = '${fparse duct_pitch_inner / 2} ${fparse duct_pitch_outer / 2}'
    duct_intervals = '1 1'
    duct_region_ids = ' ${mid_lower_refl}     ${mid_lower_refl};
                        ${mid_ht9}            ${mid_sodium};
                        ${mid_upper_na_plen}  ${mid_upper_na_plen};
                        ${mid_upper_gas_plen} ${mid_upper_gas_plen}'
    inputs = 'fuel_pin_2'
    pattern = '0 0 0 0 0 0 0 0 0;
              0 0 0 0 0 0 0 0 0 0;
             0 0 0 0 0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0 0 0 0 0;
           0 0 0 0 0 0 0 0 0 0 0 0 0;
          0 0 0 0 0 0 0 0 0 0 0 0 0 0;
         0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
       0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
         0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
          0 0 0 0 0 0 0 0 0 0 0 0 0 0;
           0 0 0 0 0 0 0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0 0 0 0 0;
             0 0 0 0 0 0 0 0 0 0 0;
              0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0'
  []
  [fuel_assembly_3]
    type = AssemblyMeshGenerator
    assembly_type = 3
    background_intervals = 1
    background_region_id = '${mid_lower_refl} ${mid_sodium} ${mid_upper_na_plen} ${mid_upper_gas_plen}'
    duct_halfpitch = '${fparse duct_pitch_inner / 2} ${fparse duct_pitch_outer / 2}'
    duct_intervals = '1 1'
    duct_region_ids = ' ${mid_lower_refl}     ${mid_lower_refl};
                        ${mid_ht9}            ${mid_sodium};
                        ${mid_upper_na_plen}  ${mid_upper_na_plen};
                        ${mid_upper_gas_plen} ${mid_upper_gas_plen}'
    inputs = 'fuel_pin_3'
    pattern = '0 0 0 0 0 0 0 0 0;
              0 0 0 0 0 0 0 0 0 0;
             0 0 0 0 0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0 0 0 0 0;
           0 0 0 0 0 0 0 0 0 0 0 0 0;
          0 0 0 0 0 0 0 0 0 0 0 0 0 0;
         0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
       0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
        0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
         0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;
          0 0 0 0 0 0 0 0 0 0 0 0 0 0;
           0 0 0 0 0 0 0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0 0 0 0 0;
             0 0 0 0 0 0 0 0 0 0 0;
              0 0 0 0 0 0 0 0 0 0;
               0 0 0 0 0 0 0 0 0'
  []
  [control_assembly]
    type = AssemblyMeshGenerator
    assembly_type = 4
    background_intervals = 1
    background_region_id = '${mid_control_empty} ${mid_control_empty} ${mid_sodium} ${mid_sodium}'
    duct_halfpitch = '${fparse control_duct_pitch_inner / 2} ${fparse control_duct_pitch_outer / 2}
                      ${fparse duct_pitch_inner / 2}         ${fparse duct_pitch_outer / 2}'
    duct_intervals = '1 1 1 1'
    duct_region_ids = '${mid_control_empty} ${mid_control_empty} ${mid_control_empty} ${mid_control_empty};
                       ${mid_control_empty} ${mid_control_empty} ${mid_control_empty} ${mid_control_empty};
                       ${mid_ht9}           ${mid_sodium}        ${mid_ht9}           ${mid_sodium};
                       ${mid_ht9}           ${mid_sodium}        ${mid_ht9}           ${mid_sodium}'
    inputs = 'control_pin'
    pattern = '0 0 0 0 0 0;
              0 0 0 0 0 0 0;
             0 0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0 0;
           0 0 0 0 0 0 0 0 0 0;
          0 0 0 0 0 0 0 0 0 0 0;
           0 0 0 0 0 0 0 0 0 0;
            0 0 0 0 0 0 0 0 0;
             0 0 0 0 0 0 0 0;
              0 0 0 0 0 0 0;
               0 0 0 0 0 0'
  []

  ### Step 3. Define homogenized assembly mesh structures to stitch into core
  [reflector_assembly]
    type = PinMeshGenerator
    reactor_params = rmp           # Name of ReactorMeshParams object
    pin_type = 5                   # Unique identiifier for assembly type
    pitch = ${assembly_pitch}      # Assembly pitch
    homogenized = true             # Set to true to define homogenized mesh structure
    use_as_assembly = true         # Set to true to treat output as assembly mesh structure, for stitching
                                   # directly into core lattice
    region_ids = '${mid_rad_refl};
                  ${mid_rad_refl};
                  ${mid_rad_refl};
                  ${mid_rad_refl}'
  []
  [shielding_assembly]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 6
    pitch = ${assembly_pitch}
    homogenized = true
    use_as_assembly = true
    region_ids = '${mid_rad_shld};
                  ${mid_rad_shld};
                  ${mid_rad_shld};
                  ${mid_rad_shld}' # Background
  []

  ### Step 4. Define final heterogeneous ABTR core mesh structure with patterned assemblies
  [het_core]
    type = CoreMeshGenerator
    inputs = 'fuel_assembly_1 fuel_assembly_2
              fuel_assembly_3 control_assembly
              reflector_assembly shielding_assembly
              dummy'                                # Name of constituent assemblies
    dummy_assembly_name = 'dummy'                   # Name of dummy assembly to
                                                    # remove from core lattice pattern
    pattern = '6 6 5 5 5 5 5 6 6;
              6 5 5 4 4 4 4 5 5 6;
             5 5 4 4 4 4 4 4 4 5 5;
            5 4 4 4 4 1 1 4 4 4 4 5;
           5 4 4 4 1 1 3 1 1 4 4 4 5;
          5 4 4 1 1 2 0 0 4 1 1 4 4 5;
         5 4 4 1 3 0 0 2 0 0 3 1 4 4 5;
        6 5 4 4 1 0 3 0 0 3 0 1 4 4 5 6;
       6 5 4 4 1 4 0 0 3 0 0 2 1 4 4 5 6;
        6 5 4 4 1 0 2 0 0 2 0 1 4 4 5 6;
         5 4 4 1 3 0 0 3 0 0 3 1 4 4 5;
          5 4 4 1 1 2 0 0 4 1 1 4 4 5;
           5 4 4 4 1 1 3 1 1 4 4 4 5;
            5 4 4 4 4 1 1 4 4 4 4 5;
             5 5 4 4 4 4 4 4 4 5 5;
              6 5 5 4 4 4 4 5 5 6;
               6 6 5 5 5 5 5 6 6'                   # Lattice pattern of constituent assemblies
    extrude = true                                  # Extrude core to 3-D
  []
[]
