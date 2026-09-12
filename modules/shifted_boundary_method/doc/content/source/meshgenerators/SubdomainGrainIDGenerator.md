# SubdomainGrainIDGenerator

!syntax description /Mesh/SubdomainGrainIDGenerator

The `SubdomainGrainIDGenerator` assigns volume-mesh subdomain IDs from the closed regions in a
saved boundary mesh. For elements intercepted by one or more regions, the selected
`intercepted_subdomain_policy` determines which grain ID is assigned. Elements outside every grain
retain their input subdomain ID.

!syntax parameters /Mesh/SubdomainGrainIDGenerator

!syntax inputs /Mesh/SubdomainGrainIDGenerator

!syntax children /Mesh/SubdomainGrainIDGenerator
