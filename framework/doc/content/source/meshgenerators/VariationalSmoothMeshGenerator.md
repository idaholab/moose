# VariationalSmoothMeshGenerator

!syntax description /Mesh/VariationalSmoothMeshGenerator

## Example

The `iterations` parameter controls the number of smoothing steps. This is an iterative process 
based on minimizing energy functionals. The optimization problem requires derivative based computations 
and an algebraic solver at each iteration step. Considering that such computations may become expensive 
on large mesh sizes, the user is advised to closely monitor the mesh quality with each iteration.

As an example here is an original mesh going through 5 iterations of the variational smoother:

!media media/mesh/smooth.gif
       id=inl-logo
       caption= To be replaced by the Vraiational Smoother.
       style=width:50%;padding:20px;

!syntax parameters /Mesh/VariationalSmoothMeshGenerator

!syntax inputs /Mesh/VariationalSmoothMeshGenerator

!syntax children /Mesh/VariationalSmoothMeshGenerator
