# DistributedRectilinearMeshGenerator

!syntax description /Mesh/DistributedRectilinearMeshGenerator

## Overview

Similar to GeneratedMeshGenerator - builds lines, rectangles and rectangular prisms.  
It differs though in the way the mesh is constructed in parallel.  
While `GeneratedMeshGenerator` creates a full copy of the mesh on every processor
- `DistributedRectilinearMeshGenerator` only creates the
elements / nodes each processor is assigned.  This makes it _much_ faster
in parallel and much leaner in memory.

!alert note
Be aware: `DistributedRectilinearMeshGenerator` turns on `parallel_type = distributed`.  
So make sure that everything you're using in your problem works with that before using this!

### More Information

`DistributedRectilinearMeshGenerator` works by first creating a "dual graph" of the element connectivity
- before ever building an elements.  
It then uses [PetscExternalPartitioner.md] to partition that graph - assigning elements to processors.  
Then, each processor can read the partition map and build only the elements that need to be on that processor.  
Final steps include adding in "ghosted" elements and making sure that boundary conditions are right.

## Example Syntax

!listing distributed_rectilinear_mesh_generator.i block=Mesh

!syntax parameters /Mesh/DistributedRectilinearMeshGenerator

!syntax inputs /Mesh/DistributedRectilinearMeshGenerator

!syntax children /Mesh/DistributedRectilinearMeshGenerator
