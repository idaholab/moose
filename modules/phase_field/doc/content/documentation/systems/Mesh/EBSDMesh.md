# EBSDMesh

!syntax description /Mesh/EBSDMesh

The mesh is generated from the EBSD information, to get an optimal reconstruction of the data. This
is accomplished in the mesh block using the EBSDMesh type. The same data file used with the EBSD
reader is used in the EBSDReader UserObject.  The mesh is created with one node per data point in the
EBSD data file. If you wish to use mesh adaptivity and allow the mesh to get coarser during the
simulation, the "uniform_refine" parameter is used to set how many times the mesh can be
coarsened. For this to work the number of elements along _each_ dimension has to be divisible by
$2^u$ where $u$ is the value of the ```uniform_refine``` parameter.

!alert warning title="uniform_refine" parameter
Contrary to other mesh objects the "uniform_refine" parameter will not affect the resolution of the
final mesh. It sets the levels of coarsening that can be applied to the EBSD data.

!syntax parameters /Mesh/EBSDMesh

!syntax inputs /Mesh/EBSDMesh

!syntax children /Mesh/EBSDMesh
