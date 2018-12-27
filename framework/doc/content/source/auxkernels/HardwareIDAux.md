# HardwareIDAux

!syntax description /AuxKernels/HardwareIDAux

## Description

One of the main purposes of this object is to aid in the diagnostic of mesh partitioners.  One metric to look at for mesh partitioners is how well they keep down inter-node (compute node) communication.  `HardwareIDAux` allows you to visually see the mapping of elements to compute nodes in your job.

This is particularly interesting in the case of the [PetscExternalPartitioner](PetscExternalPartitioner.md) which has the capability to do "hierarchical" partitioning.  Hierarchical partitioning makes it possible to partition over compute-nodes first... then within compute nodes, in order to better respect the physical topology of the compute cluster.

One important aspect of that is that how you launch your parallel job can matter quite a bit to partitioning.  In-general, it's better for partitioners if all of the ranks of your job are contiguously assigned to each compute node.  Here are four different ways, and the outcome using `HardwareIDAux`, to launch a job using a 100x100 generated mesh on 16 processes and 4 ndoes with two different partitioner...

Top left (METIS):

```
mpiexec -n 16 -host lemhi0002,lemhi0003,lemhi0004,lemhi0005 ../../../moose_test-opt -i hardware_id_aux.i
```

Top right (Hierarchic):

```
mpiexec -n 16 -host lemhi0002,lemhi0003,lemhi0004,lemhi0005 ../../../moose_test-opt -i hardware_id_aux.i -mat_partitioning_hierarchical_nfineparts 4
```

Bottom left (METIS):

```
mpiexec -n 16 -host lemhi0002,lemhi0003,lemhi0004,lemhi0005 -ppn 4 ../../../moose_test-opt -i hardware_id_aux.i
```

Bottom right (Hierarchic):

```
mpiexec -n 16 -host lemhi0002,lemhi0003,lemhi0004,lemhi0005 -ppn 4 ../../../moose_test-opt -i hardware_id_aux.i -mat_partitioning_hierarchical_nfineparts 4
```

It should be immediately apparent that the bottom right partitioning is best (will reduce the amount of inter-node communication).  That result was achieved by using hierarchical partitioning and using `-ppn 4` to tell `mpiexec` to put `4` processes on each compute node... which will cause those four processes to be contiguous on each node.  The top two examples, which omit the `-ppn` option, end up getting "striped" mpi processes (one process is placed on each node and then it wraps around) causing a jumbly mess of partitioning which will increase the communication cost for the job (and decrease scalability).

!media media/auxkernels/hardware_id_aux.png style=width:75%

!syntax parameters /AuxKernels/HardwareIDAux

!syntax inputs /AuxKernels/HardwareIDAux

!syntax children /AuxKernels/HardwareIDAux

!bibtex bibliography
