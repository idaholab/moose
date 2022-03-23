# SidesetsEncloseBlocks

!syntax description /Mesh/SidesetsEncloseBlocks

This mesh generator ensures that the blocks provided by the
[!param](/meshgenerators/SidesetsEncloseBlocks/block)
parameter are  enclosed by the sidesets provided by [!param](/meshgenerators/SidesetsEncloseBlocks/boundary).
If that is not the case, the behavior of the mesh generator depends on whether
[!param](/meshgenerators/SidesetsEncloseBlocks/new_boundary) is provided.
If it is not provided, then an error is thrown. If it is provided, then the side
that is not covered is added to the new boundary.

!syntax parameters /Mesh/SidesetsEncloseBlocks

!syntax inputs /Mesh/SidesetsEncloseBlocks

!syntax children /Mesh/SidesetsEncloseBlocks
