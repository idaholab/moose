# SmoothMeshGenerator

!syntax description /Mesh/SmoothMeshGenerator

## Example

The `iterations` parameter controls the number of smoothing steps to do.  Each
smoothing step will iterate the mesh toward the "true" smoothed mesh (as
measured by the Laplacian smoother).  Note that the mesh should reach "steady
state": after just a few iterations the mesh will stop moving by much.

As an example here is an original mesh going through 12 iterations of this smoother:

!media media/mesh/smooth.gif
       id=inl-logo
       caption=12 iterations of Laplacian smoothing.  Coloring is by element quality (higher is better).
       style=width:50%;padding:20px;

!syntax parameters /Mesh/SmoothMeshGenerator

!syntax inputs /Mesh/SmoothMeshGenerator

!syntax children /Mesh/SmoothMeshGenerator
