# Shifted Boundary Method Module

The Shifted Boundary Method (SBM) module provides a flexible framework for solving
partial differential equations on complex geometries without requiring boundary-fitted
meshes. Instead of conforming the mesh to curved or intricate boundaries, SBM embeds
the geometry inside a background mesh and shifts the boundary conditions from the
true interface to surrogate locations on nearby mesh faces. This approach preserves
accuracy, avoids costly remeshing, and enables robust treatment of Dirichlet, Neumann,
and Robin conditions.

The module includes implementations for distance and normal evaluation, as well as
surrogate boundary integration. It also provides interfaces to boundary-representation–
based distance and normal calculations, which can be accessed by other physics
modules such as fluid flow, heat transfer, and structural mechanics.

By avoiding mesh regeneration, the SBM module significantly simplifies simulations
involving moving interfaces, complex solid boundaries, and multiphysics interactions.
