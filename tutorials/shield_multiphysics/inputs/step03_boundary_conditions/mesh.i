!include ../step01_diffusion/mesh.i

[Mesh]
  [add_concrete_outer_boundary]
    type = RenameBoundaryGenerator
    input = rename_blocks
    old_boundary = 'left right front bottom top back'
    new_boundary = 'air_boundary air_boundary air_boundary air_boundary air_boundary ground'
  []
  [add_water_concrete_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_concrete_outer_boundary
    primary_block = 'water water water'
    paired_block = 'concrete_hd concrete Al'
    new_boundary = 'water_boundary'
  []
  [add_water_concrete_interface_inwards]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_water_concrete_interface
    primary_block = 'concrete_hd concrete Al'
    paired_block = 'water water water'
    new_boundary = 'water_boundary_inwards'
  []
  [add_inner_cavity_solid]
    type = SideSetsAroundSubdomainGenerator
    input = add_water_concrete_interface_inwards
    block = Al
    new_boundary = 'inner_cavity_solid'
    include_only_external_sides = true
  []
  [add_inner_cavity_water]
    type = SideSetsAroundSubdomainGenerator
    input = add_inner_cavity_solid
    block = water
    new_boundary = 'inner_cavity_water'
    include_only_external_sides = true
  []
[]

