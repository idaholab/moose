# Relationship Managers

The RelationshipManager system is for making MOOSE aware of extra Geometric or Algebraic information needed to perform
a calculation. This is an extension of the "stencil" concept when working with a structured mesh and performing
calculations where you need neighboring information. Relationship managers (RMs) come in two flavors: Geometric and Algebraic.
Geometric RMs are useful for making sure extra mesh elements beyond your individual processor partitions are available.
In DistributedMesh mode, you can only count on having one extra layer of neighboring elements available outside of your
partition.

The RM system is fully pluggable like other MOOSE systems but it not intended to be exposed in the input file
in any way. Instead, individual objects that require extra information should "register" an appropriate RM right in
its `validParams` function and have MOOSE add the appropriate object to the simulation at the proper time. It is
possible for an RM to supply by Geometric and Algebraic information.

!alert note
Geometric RMs are not necessary (active) when running MOOSE in ReplicatedMesh mode.

## Geometric Relationship Managers

Geometric Relationship Managers are useful if a user requires extra "halo" information around a partition. Use cases may
include Reconstructed Discontinous Galerkin (rDG) or the use of the GrainTracker in the PhaseField module.

## Algebraic Relationship Managers

Algebraic Relationship Magers are useful if an objects requires extra solution or field information from elements
beyond the current processor's partition.
