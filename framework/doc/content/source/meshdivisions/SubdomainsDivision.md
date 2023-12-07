# SubdomainsDivision

!syntax description /MeshDivisions/SubdomainsDivision

If not setting the [!param](/MeshDivisions/SubdomainsDivision/block) parameter:
The divisions are numbered from 0 to the number of blocks in the mesh minus one.
The ordering of the divisions is the same of the ordering of the IDs of the subdomains.

If specifying a block restriction using the [!param](/MeshDivisions/SubdomainsDivision/block) parameter:
The divisions are numbered from 0 to the number of blocks specified minus one. An invalid id is returned
for points and elements outside the block restriction. The order of the divisions is the same
as the ordering of the blocks in the [!param](/MeshDivisions/SubdomainsDivision/block) parameter.

!syntax parameters /MeshDivisions/SubdomainsDivision

!syntax inputs /MeshDivisions/SubdomainsDivision

!syntax children /MeshDivisions/SubdomainsDivision
