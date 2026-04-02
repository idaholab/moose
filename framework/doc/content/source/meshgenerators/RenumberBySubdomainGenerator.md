# RenumberBySubdomainGenerator

!syntax description /Mesh/RenumberBySubdomainGenerator

!alert note
Because nodes are shared between multiple subdomains at internal boundaries between in the mesh, the node
numbering will in general only be contiguous for nodes that are inside the subdomain, away from these boundaries.

!alert warning
Renumbering elements and nodes will prevent using [Exodus.md] restart capabilities as well as using the [!param](/AuxKernels/SolutionAux/direct)
parameter of the [SolutionAux.md], as they both rely on the source file mesh having the same numbering as the current mesh.

!alert note
The current implementation only guarantees contiguous numbering of elements in each subdomain if all subdomains are being
renumbered.

!syntax parameters /Mesh/RenumberBySubdomainGenerator

!syntax inputs /Mesh/RenumberBySubdomainGenerator

!syntax children /Mesh/RenumberBySubdomainGenerator
