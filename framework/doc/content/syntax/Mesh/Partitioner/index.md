# Partitioner System

The Partitioner System allows the developer to control the partioning process
to split up a mesh among two or more processors. There are several partitioners
available in both PETSc and libMesh which can be directly dialed up through the
MOOSE input file via the [/LibmeshPartitioner.md] and [/PetscExternalPartitioner.md]
objects. Custom Partitioners may be built by inherting from MoosePartitioner.md
and overridding the special `_do_parition()` method (from libMesh's partitioner).

!syntax list /Mesh/Partitioner objects=True actions=False subsystems=False

!syntax list /Mesh/Partitioner objects=False actions=False subsystems=True

!syntax list /Mesh/Partitioner objects=False actions=True subsystems=False
