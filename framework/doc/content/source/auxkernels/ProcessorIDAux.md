# ProcessorIDAux

Auxiliary kernel for displaying mesh partitioning. Each node or element can display its corresponding processor ID.

!media media/framework/auxkernels/partition.png caption=Coarse regular mesh partitioned 5 ways.

!alert note
This AuxKernel should be used with care in regression tests. Partitioning is often different between
different platforms and clearly running on a different numbers of processors will change this field
substantially.

!syntax description /AuxKernels/ProcessorIDAux

!syntax parameters /AuxKernels/ProcessorIDAux

!syntax inputs /AuxKernels/ProcessorIDAux

!syntax children /AuxKernels/ProcessorIDAux
