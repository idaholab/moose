# Empire Reactor - 3D Core Microreactor Core using Reactor Geometry Mesh Builder (RGMB)

# ==============================================================================
# Global Variables
# ==============================================================================
# These global variables assign a region ID to each region in the core
# RGMB uses these region IDs to assign the `region_id` extra element integer

mid_fuel_1 = 1      # Fuel region of fuel_pin_1
mid_fuel_2 = 2      # Fuel region of fuel_pin_2
mid_fuel_3 = 3      # Fuel region of fuel_pin_3
mid_fgap = 4        # Gap region of fuel pins
mid_mgap = 5        # Gap region of moderator pin
mid_moderator = 6   # Moderator region of moderator pin
mid_hpipe = 7       # Heat pipe region of heat pipe pin
mid_ss = 8          # Structural stainless steel region
mid_air = 9         # Air region of air hole assembly
mid_reflector = 10  # Reflector region
mid_drum_pad = 11   # Drum pad region of control drum assembly

[Mesh]

  #################################      # This parameter allows us to execute the file but stop at this block so we can see intermediate output.
  final_generator = core                 # User: Change this to 'fuel_pin_1', 'fuel_assembly_1', or 'airhole_assembly', 'cd_ne', or 'core'
  #################################

  # step 1: fuel_pin_1 (pin mesh structure to stitch into fuel assemblies)
  # step 2: fuel_assembly_1 (assembly mesh structure to define fuel assemblies)
  # step 3: airhole_assembly (homogenized assembly mesh structure to stitch into core)
  # step 4: cd_ne (control drum structure to stitch into core)
  # step 5: core (final Empire core mesh structure)

  ### Step 0. Define global parameters for Reactor Geometry Mesh Builder workflow
              # Note: This step does not produce any mesh
  [rmp]
    type = ReactorMeshParams
    dim = 2                            # Dimensionality of output mesh (2 or 3)
    geom = "Hex"                       # Geometry type (Hex or Square)
    assembly_pitch = 32.353            # Size of assembly flat-to-flat pitch
    radial_boundary_id = 203           # Boundary id assigned to radial surface
    flexible_assembly_stitching = true # Set to true to stitch dissimilar assembly types together,
                                       # i.e. homogeneous, heterogeneous, and control drum assemblies
    region_id_as_block_name = true     # Set this to true to assign block name based on the region ID
                                       # of the region. If quad and tri elements share the same region ID
                                       # two separate block names will be created
  []

  ### Step 1. Define pin mesh structures to stitch into fuel assemblies
  [fuel_pin_1]
    type = PinMeshGenerator
    reactor_params = rmp               # Name of ReactorMeshParams object
    pin_type = 1                       # Unique identifier for pin type
    pitch = 2.15                       # Pin pitch
    num_sectors = 2                    # Number of azimuthal sectors per hexagonal side
    quad_center_elements = false       # Whether central mesh elements in inner ring should use
                                       # quad elements (true) or tri elements (false)
    ring_radii = '0.925 0.975'         # Radii for each ring
    mesh_intervals = '3 1 1'           # Number of radial intervals for each radial region
                                       # (inner ring, outer ring, background)
    region_ids = '${mid_fuel_1} ${mid_fgap} ${mid_ss}' # Region IDs for each radial region
                                                       # (inner ring, outer ring, background)
  []
  [fuel_pin_2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 2.15
    num_sectors = 2
    quad_center_elements = false
    ring_radii = '0.925 0.975'
    mesh_intervals = '3 1 1'    # Fuel, gap, background
    region_ids = '${mid_fuel_2} ${mid_fgap} ${mid_ss}'
  []
  [fuel_pin_3]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = 2.15
    num_sectors = 2
    quad_center_elements = false
    ring_radii = '0.925 0.975'
    mesh_intervals = '3 1 1'    # fuel, gap, background
    region_ids = '${mid_fuel_3} ${mid_fgap} ${mid_ss}'
  []
  [hpipe_pin]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 6
    pitch = 2.15
    num_sectors = 2
    quad_center_elements = false
    ring_radii = '1.0'
    mesh_intervals = '3 1'    # pin, background
    region_ids = '${mid_hpipe} ${mid_ss}'
  []
  [mod_pin]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 7
    pitch = 2.15
    num_sectors = 2
    quad_center_elements = false
    ring_radii = '0.95 1.0'
    mesh_intervals = '3 1 1'    # pin, gap, background
    region_ids = '${mid_moderator} ${mid_mgap} ${mid_ss}'
  []

  ### Step 2. Define assembly mesh structures for fuel assemblies
  [fuel_assembly_1]
    type = AssemblyMeshGenerator
    assembly_type = 1                       # Unique identifier for pin type
    background_intervals = 1                # Number of radial intervals in background region
    background_region_id = '${mid_ss}'      # Region ID corresponding to background region
    inputs = 'fuel_pin_1 hpipe_pin mod_pin' # Name of contituent pin mesh structures
    pattern = '1 0 1 0 1 0 1 0 1;
              0 2 2 2 2 2 2 2 2 0;
             1 2 1 0 1 0 1 0 1 2 1;
            0 2 0 2 2 2 2 2 2 0 2 0;
           1 2 1 2 1 0 1 0 1 2 1 2 1;
          0 2 0 2 0 2 2 2 2 0 2 0 2 0;
         1 2 1 2 1 2 1 0 1 2 1 2 1 2 1;
        0 2 0 2 0 2 0 2 2 0 2 0 2 0 2 0;
       1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1;
        0 2 0 2 0 2 0 2 2 0 2 0 2 0 2 0;
         1 2 1 2 1 2 1 0 1 2 1 2 1 2 1;
          0 2 0 2 0 2 2 2 2 0 2 0 2 0;
           1 2 1 2 1 0 1 0 1 2 1 2 1;
            0 2 0 2 2 2 2 2 2 0 2 0;
             1 2 1 0 1 0 1 0 1 2 1;
              0 2 2 2 2 2 2 2 2 0;
               1 0 1 0 1 0 1 0 1'           # Lattice pattern of constituent pins
  []
  [fuel_assembly_2]
    type = AssemblyMeshGenerator
    assembly_type = 2
    background_intervals = 1
    background_region_id = '${mid_ss}'
    inputs = 'fuel_pin_2 hpipe_pin mod_pin'
    pattern = '1 0 1 0 1 0 1 0 1;
              0 2 2 2 2 2 2 2 2 0;
             1 2 1 0 1 0 1 0 1 2 1;
            0 2 0 2 2 2 2 2 2 0 2 0;
           1 2 1 2 1 0 1 0 1 2 1 2 1;
          0 2 0 2 0 2 2 2 2 0 2 0 2 0;
         1 2 1 2 1 2 1 0 1 2 1 2 1 2 1;
        0 2 0 2 0 2 0 2 2 0 2 0 2 0 2 0;
       1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1;
        0 2 0 2 0 2 0 2 2 0 2 0 2 0 2 0;
         1 2 1 2 1 2 1 0 1 2 1 2 1 2 1;
          0 2 0 2 0 2 2 2 2 0 2 0 2 0;
           1 2 1 2 1 0 1 0 1 2 1 2 1;
            0 2 0 2 2 2 2 2 2 0 2 0;
             1 2 1 0 1 0 1 0 1 2 1;
              0 2 2 2 2 2 2 2 2 0;
               1 0 1 0 1 0 1 0 1'
  []
  [fuel_assembly_3]
    type = AssemblyMeshGenerator
    assembly_type = 3
    background_intervals = 1
    background_region_id = '${mid_ss}'
    inputs = 'fuel_pin_3 hpipe_pin mod_pin'
    pattern = '1 0 1 0 1 0 1 0 1;
              0 2 2 2 2 2 2 2 2 0;
             1 2 1 0 1 0 1 0 1 2 1;
            0 2 0 2 2 2 2 2 2 0 2 0;
           1 2 1 2 1 0 1 0 1 2 1 2 1;
          0 2 0 2 0 2 2 2 2 0 2 0 2 0;
         1 2 1 2 1 2 1 0 1 2 1 2 1 2 1;
        0 2 0 2 0 2 0 2 2 0 2 0 2 0 2 0;
       1 2 1 2 1 2 1 2 1 2 1 2 1 2 1 2 1;
        0 2 0 2 0 2 0 2 2 0 2 0 2 0 2 0;
         1 2 1 2 1 2 1 0 1 2 1 2 1 2 1;
          0 2 0 2 0 2 2 2 2 0 2 0 2 0;
           1 2 1 2 1 0 1 0 1 2 1 2 1;
            0 2 0 2 2 2 2 2 2 0 2 0;
             1 2 1 0 1 0 1 0 1 2 1;
              0 2 2 2 2 2 2 2 2 0;
               1 0 1 0 1 0 1 0 1'
  []

  ### Step 3. Define homogenized assembly mesh structures to stitch into core
  [airhole_assembly]
    type = PinMeshGenerator
    reactor_params = rmp               # Name of ReactorMeshParams object
    pin_type = 4                       # Unique identiifier for assembly type
    pitch = 32.353                     # Assembly pitch
    region_ids = '${mid_air}'          # Region ID for homogenized region
    homogenized = true                 # Set to true to define homogenized mesh structure
    use_as_assembly = true             # Set to true to treat output as assembly mesh structure, for stitching
                                       # directly into core lattice
  []
  [refl_assembly]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 5
    pitch = 32.353
    region_ids = '${mid_reflector}'
    homogenized = true
    use_as_assembly = true
  []

  ### Step 4. Define control drum structures to stitch into core
  [cd_ne]
    type = ControlDrumMeshGenerator
    assembly_type = 6                  # Unique identifier for assembly type
    reactor_params = rmp               # Name of ReactorMeshParams object
    drum_inner_radius = 13.8           # Inner radius of drum region
    drum_outer_radius = 14.8           # Outer radius of drum region
    num_azimuthal_sectors = 72         # Number of azimuthal sectors for
    drum_inner_intervals = 15          # Number of radial intervals in drum inner region
    drum_intervals = 3                 # Number of radial intervals in drum region
    pad_start_angle = 15               # Starting angle of drum pad region
    pad_end_angle = 105                # Ending angle of drum pad region
    region_ids = '${mid_reflector} ${mid_drum_pad}
                  ${mid_reflector} ${mid_reflector}' # Region IDs of control drum region
                                                     # (drum inner, drum pad, drum ex-pad, background)
  []
  [cd_se]
    type = ControlDrumMeshGenerator
    assembly_type = 7
    reactor_params = rmp
    drum_inner_radius = 13.8
    drum_outer_radius = 14.8
    num_azimuthal_sectors = 72
    drum_inner_intervals = 15
    drum_intervals = 3
    pad_start_angle = 75
    pad_end_angle = 165
    region_ids = '${mid_reflector} ${mid_drum_pad} ${mid_reflector} ${mid_reflector}'
  []
  [cd_s]
    type = ControlDrumMeshGenerator
    assembly_type = 8
    reactor_params = rmp
    drum_inner_radius = 13.8
    drum_outer_radius = 14.8
    num_azimuthal_sectors = 72
    drum_inner_intervals = 15
    drum_intervals = 3
    pad_start_angle = 135
    pad_end_angle = 225
    region_ids = '${mid_reflector} ${mid_drum_pad} ${mid_reflector} ${mid_reflector}'
  []
  [cd_sw]
    type = ControlDrumMeshGenerator
    assembly_type = 9
    reactor_params = rmp
    drum_inner_radius = 13.8
    drum_outer_radius = 14.8
    num_azimuthal_sectors = 72
    drum_inner_intervals = 15
    drum_intervals = 3
    pad_start_angle = 195
    pad_end_angle = 285
    region_ids = '${mid_reflector} ${mid_drum_pad} ${mid_reflector} ${mid_reflector}'
  []
  [cd_nw]
    type = ControlDrumMeshGenerator
    assembly_type = 10
    reactor_params = rmp
    drum_inner_radius = 13.8
    drum_outer_radius = 14.8
    num_azimuthal_sectors = 72
    drum_inner_intervals = 15
    drum_intervals = 3
    pad_start_angle = 255
    pad_end_angle = 345
    region_ids = '${mid_reflector} ${mid_drum_pad} ${mid_reflector} ${mid_reflector}'
  []
  [cd_n]
    type = ControlDrumMeshGenerator
    assembly_type = 11
    reactor_params = rmp
    drum_inner_radius = 13.8
    drum_outer_radius = 14.8
    num_azimuthal_sectors = 72
    drum_inner_intervals = 15
    drum_intervals = 3
    pad_start_angle = 315
    pad_end_angle = 405
    region_ids = '${mid_reflector} ${mid_drum_pad} ${mid_reflector} ${mid_reflector}'
  []

  ### Step 5. Define final Empire core mesh structure
  [core]
    type = CoreMeshGenerator
    inputs = 'fuel_assembly_1 fuel_assembly_2
              fuel_assembly_3 airhole_assembly
              refl_assembly cd_ne cd_se cd_s cd_sw
              cd_nw cd_n'                           # Name of constituent assemblies
    pattern = '4  4  4  4  4;
              4  4 10 10  4  4;
             4  9  1  2  1  5  4;
            4  9  2  0  0  2  5  4;
           4  4  1  0  3  0  1  4  4;
            4  8  2  0  0  2  6  4;
             4  8  1  2  1  6  4;
              4  4  7  7  4  4;
               4  4  4  4  4'                       # Lattice pattern of constituent assemblies

    # Define depletion IDs for each unique (pin, region_id) pair
    generate_depletion_id = true
    depletion_id_type = pin
  []
[]
