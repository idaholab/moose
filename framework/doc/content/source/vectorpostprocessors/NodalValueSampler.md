# NodalValueSampler

!syntax description /VectorPostprocessors/NodalValueSampler

This `VectorPostprocessor` is used for sampling nodal variables at each node
in the domain, selection of blocks, or selection of boundaries.

!alert note title=Vector names / CSV output column names
`NodalValueSampler` declares a vector for each spatial coordinate, (`x`, `y`, `z`), of the sampled nodes,
the IDs of the nodes in a vector named `id`,
and a vector named after each variable sampled, containing the variable values at each point.

!syntax parameters /VectorPostprocessors/NodalValueSampler

!syntax inputs /VectorPostprocessors/NodalValueSampler

!syntax children /VectorPostprocessors/NodalValueSampler
