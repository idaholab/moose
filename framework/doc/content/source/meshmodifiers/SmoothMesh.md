# SmoothMesh

!syntax description /MeshModifiers/SmoothMesh

## Example

The `iterations` parameter controls the number of smoothing steps to do.  Each smoothing step will iterate the mesh toward the "true" smoothed mesh (as measured by the Laplacian smoother).  Note that the mesh should reach "steady state": after just a few iterations the mesh will stop moving by much.

As an example here is a non-smoothed mesh:

!syntax parameters /MeshModifiers/SmoothMesh

!syntax inputs /MeshModifiers/SmoothMesh

!syntax children /MeshModifiers/SmoothMesh

!bibtex bibliography
