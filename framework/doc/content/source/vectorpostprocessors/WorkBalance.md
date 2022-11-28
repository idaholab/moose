# WorkBalance

## Short Description

!syntax description /VectorPostprocessors/WorkBalance

## Description

The idea here is to compute per-processor metrics to help in determining the quality of a partitioning.

Currently computes: number of local elements, nodes, dofs and partition sides.  The partition sides are the sides of elements that are on processor boundaries (also known as the "edge-cuts" in partitioner lingo).  Also computes the "surface area" of each partition (physically, how much processor boundary each partitioning has).

!alert note title=Vector names
The names of the vectors declared by `WorkBalance` are the names of the [!param](/VectorPostprocessors/WorkBalance/balances) requested.

### HardwareID

`WorkBalance` will now also compute the number of sides and the surface area for the partition on each compute node (called "hardware_id" here) in the cluster.  This gives the amount of "inter-node" communication.  Use of a hierarchical partitioner (like the one available in [PetscExternalPartitioner](PetscExternalPartitioner.md)) can help reduce inter-node communication.

For instance, here is a 1600x1600 mesh partitioned to run on 64 nodes, each having 36 processors (2304 processors total).  Using `WorkBalance` and [VectorPostprocessorVisualizationAux](VectorPostprocessorVisualizationAux.md) we can visually see how much inter-node communication there is and quantify it.

!media media/vectorpostprocessors/work_balance_hardware_id.png style=width:75% caption=Visualization of inter-node communication. Left: Parmetis, Right: Hierarchical.  Parmetis `hardware_id_surface_area`: 66.  Hierarchical `hardware_id_surface_area`: 39.

!syntax parameters /VectorPostprocessors/WorkBalance

!syntax inputs /VectorPostprocessors/WorkBalance

!syntax children /VectorPostprocessors/WorkBalance
