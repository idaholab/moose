# GeochemistrySpatialReactor

This UserObject is usually added via a [SpatialReactionSolver](SpatialReactionSolver/index.md) Action: please see that page for many examples.  This UserObject's purpose is to solve a time-and-space dependent geochemical system.

Advanced users may also add `GeochemistrySpatialReactor` objects manually to their input files

## Block restriction id=block-restriction

A `GeochemistrySpatialReactor` may be restricted to part of the mesh using the `block` parameter, for instance to give each facies of a model its own mineralogy.  The reactor then solves and time-steps only the nodes of elements in those blocks, and holds no chemistry elsewhere, so any object that queries it, such as a [GeochemistryQuantityAux](GeochemistryQuantityAux.md), must carry the same block restriction: querying a node outside the blocks is an error.  The [SpatialReactionSolver](SpatialReactionSolver/index.md) Action restricts the AuxVariables and AuxKernels it adds automatically.

The SpatialReactionSolver Action adds a single reactor, so to give different blocks different chemistry, add one `GeochemistrySpatialReactor` per block manually, together with block-restricted AuxVariables and AuxKernels, for example:

!listing modules/geochemistry/test/tests/spatial_reactor/two_facies.i block=UserObjects AuxVariables AuxKernels

!alert warning title=Nodes shared between blocks
A node on the interface between two blocks belongs to both.  A single reactor restricted to several blocks time-steps such a node once per time step, but reactors restricted to adjoining blocks each hold their own chemistry there.  Without transport the two chemistries simply evolve independently, but when the reactors are coupled to transport, the transported species have a single value at that node while the two reactors hold and report different ones.  If the AuxKernels querying the two reactors write to the same AuxVariable, its value at such a node is the one computed by the AuxKernel on the block with the highest subdomain ID, whatever the order of the objects in the input file.  In a model with lower-dimensional fractures that share their nodes with the surrounding matrix, every fracture node is such a node: a single reactor restricted to the fracture blocks is appropriate when the matrix is unreactive, but a second reactor on the matrix blocks would also act at every fracture node.

!syntax parameters /UserObjects/GeochemistrySpatialReactor

!syntax inputs /UserObjects/GeochemistrySpatialReactor

!syntax children /UserObjects/GeochemistrySpatialReactor

