# CrackMeshCut3DUserObject

!syntax description /UserObjects/CrackMeshCut3DUserObject

## Overview

This class: (1) reads in a mesh describing the crack surface, (2) uses the mesh to do initial cutting of 3D elements, and (3) grows the mesh incrementally based on prescribed growth functions. The code is interfaced with domain integral methods to allow nonplanar crack growth based on empirical propagation direction and speed laws.

For crack-growth problems, [!param](/UserObjects/CrackMeshCut3DUserObject/size_control) sets the target spacing used when advancing and refining the cutter front, while[!param](/UserObjects/CrackMeshCut3DUserObject/min_elem_area) prevents the growth algorithm from adding very small or nearly degenerate triangles to the cutter mesh. The number of growth advances performed in one update is controlled by [!param](/UserObjects/CrackMeshCut3DUserObject/n_step_growth).

## Surface-Cutting Segment, Active and Inactive Nodes

When a crack reaches a free surface of the body, only part of the cutter-mesh boundary lies inside the FEM volume. The cutter boundary is therefore partitioned into one or more *active boundary segments*, each consisting of a contiguous list of nodes bracketed by *inactive endpoints*:

```
inactive_start — active_1 — active_2 — ... — active_N — inactive_end
```

Classification is done on every growth step by testing whether each cutter-boundary node lies inside the FEM body:

- A boundary node is **active** if it lies inside the body.
- A boundary node is **inactive** if it lies outside the body.

The crack front used by [DomainIntegralAction.md] and the propagation laws is composed only of active nodes. The inactive endpoints act as anchors for the active segment and define where the crack front meets the free surface.

## Motion of the Inactive Node

Each active node advances in the direction and by the increment determined by the growth law (function-prescribed or hoop-stress-based). The inactive endpoints are not driven by their own growth law; instead they inherit growth direction and increment from the active neighbor they are connected to.  So the candidate position for an inactive endpoint is

```
candidate = previous_inactive_position + active_direction * active_growth_length
```

### Keeping the inactive node above the free surface

After computing the candidate, the algorithm checks whether it still lies outside the body:

1. **Candidate outside the body.** The inactive node has moved into free space and no correction is required; the candidate is used as the new position.
2. **Candidate inside the body.** Growth would push the inactive node into the FEM volume (typical when the free surface curves toward the cutter or when the cutter enters a concave corner). The candidate is snapped to the closest point on the body's exterior surface and then nudged outward by `0.1 * size_control` so the result lies reliably above the free surface for the next step's classification.

For every face on the body's exterior, the algorithm computes the point on that face closest to the candidate. This is the perpendicular projection of the candidate onto the face's plane when that projection lies within the face's polygon; otherwise it is the nearest point on one of the polygon's edges. The face whose closest point is nearest to the candidate is selected, and that point becomes the snap location. The same rule covers two important cases without special handling:

- **Curved free surfaces.** Because the curved surface is tiled by many small flat polygons, the candidate naturally snaps to the polygon of the tiling that is closest to it; the snap point varies smoothly across the curvature.
- **Concave corners.** When the candidate is just past a corner edge shared between two adjacent faces, the edge-fallback gives the same point on that shared edge for both faces. The outward nudge then pushes the inactive node into the empty space outside the corner rather than burying it back inside either face.

### Minimum-displacement reuse

After the projection step, the inactive endpoint's actual displacement from its previous position to the new position is compared against half of the active neighbor's growth length. If

```
||new_position - previous_inactive_position|| < 0.5 * active_growth_length
```

the inactive node is left at its current location and the existing node is reused for the new crack-front element. This avoids producing degenerate sliver triangles when projection clips the inactive motion almost back to where it started (for example, when an inactive endpoint is pressed against a wall and growth would only push it deeper). When this rule fires, the connection between the old front and the new front shares a node at that endpoint, and the zero-area sliver triangle that would otherwise be generated is dropped by the existing minimum-area filter.

!alert note
The closest-point projection only considers surfaces tagged into a sideset. A free surface that exists geometrically but is not assigned to any sideset will be invisible to this projection, and an inactive endpoint pushed into the body through such an untagged surface will not be corrected. When meshing for crack-growth simulations, every exterior face that the cutter can interact with should be assigned to a sideset.

!alert note
This active/inactive logic governs only the cutter-mesh growth algorithm — that is, how the cutter advances and where its free-surface endpoints are placed each step. The XFEM cutting itself is unaffected: every FEM element intersected by the cutter mesh is cut normally regardless of whether the intersection happens near an active interior, an inactive endpoint, or a reused node.

## Example Input Syntax

This example shows the `Mesh` block in [list:mesh] needed for creating the cutter mesh along with the `CrackMeshCut3DUserObject` block in [list:cutobject].  The mesh block in [list:mesh] defines two separate meshes.  The cutter mesh is created in the `read_in_cutter_mesh` block and must have [!param](/Mesh/FileMeshGenerator/save_with_name) set in order to specify this mesh in `CrackMeshCut3DUserObject` shown in [list:cutobject] using [!param](/UserObjects/CrackMeshCut3DUserObject/mesh_generator_name).  The mesh used by the FEM simulation is specifed in the `FEM_mesh` block in this example and [!param](/Mesh/MeshGeneratorMesh/final_generator)`=FEM_mesh` must be set because only the `FEM_mesh` will be used for the finite-element solution and the mesh created by `read_in_cutter_mesh` will be ignored by the solution.

!listing test/tests/solid_mechanics_basic/edge_crack_3d_domain.i id=list:mesh block=Mesh caption=Setting up the mesh block contain simulation and cutter meshes.

!listing test/tests/solid_mechanics_basic/edge_crack_3d_mhs.i id=list:cutobject block=UserObjects caption=CrackMeshCut3DUserObject that uses the cutter mesh created in [list:cutobject].

!syntax parameters /UserObjects/CrackMeshCut3DUserObject

!syntax inputs /UserObjects/CrackMeshCut3DUserObject

!syntax children /UserObjects/CrackMeshCut3DUserObject
