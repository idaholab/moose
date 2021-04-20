# Modeling Embedded Interface

In addition to modeling crack propagation, the MOOSE XFEM module can also be used to represent embedded interfaces.

In general, the interface locations are defined by individual objects (`GeometricCutUserObject`s) that define the interface location, which could be done using level sets or other geometric techniques. Each partitions the domain into a pair of *cut subdomains*. Each cut subdomain has a unique *cut subdomain ID*. Multiple pairs of cut subdomains can be mapped onto a set of unique cut subdomains based on a user-specified dictionary.

XFEM is used to insert strong discontinuities along each interface. If desired, solution continuity across the interfaces can be enforced using either a penalty constraint or the Nitsche's method.  

## Moving interface

To represent the movement of the interface, the user only needs to update the interface location defined by the corresponding `GeometricCutUserObject`. The XFEM module handles the update of the interface location using the so-called *healing-and-re-cut* algorithm. In a nutshell, every time a interface location is updated, its corresponding cut (as well as the underlying strong discontinuity) is completed removed (thereby termed with "healing"), and the updated interface is marked and re-cut following the [mesh cutting algorithm](xfem/theory/theory.md#mesh_cutting_alg) using the updated interface location.

During the healing-and-re-cut process, history-dependent data is preserved. Before an interface is "healed", stateful material properties on all children elements on the interface are cached. If the same parents are immediately re-cut in the same time step, stateful material properties on the children elements will be restored using the cache. The algorithmic flow is detailed below.

### Algorithmic flow

The data structure used to store information about a child element is hereinafter referred to as `CutElemInfo` (`CEI`). The `CEI` consists of four components: (1) the child element, (2) the parent element, (3) the `GeometricCutUserObject`, and (4) the cut subdomain ID. These information are useful when restoring cached stateful material properties.

Suppose:

- There are two elements in the mesh, with element IDs 1 and 2, respectively;
- We set `max_xfem_update = 1`, i.e. the XFEM is updated and the system is solved again only once after the first solve in each time step;
- There is one time-dependent signed distance function defining the interface. The left of the interface has cut subdomain 1, and the right of the interface has cut subdomain 2. The interface moves towards right over time.

The meshes and the `CEI` data structures are shown in [healing_recut_1], [healing_recut_2] and [healing_recut_3], for the first, the second, and the third time step, respectively.

!media media/xfem/healing_recut_1.png
       id=healing_recut_1
       style=width:100%;padding:20px;
       caption=The *healing-and-re-cut* algorithm at time step 1. For demonstration purposes, the `CEI`s are denoted using element IDs and the cut ID, while in the actual implementation pointers are used to store these information.

During the first solve in the first time step, the mesh is not yet cut by XFEM, and the `CEI`s are empty. Therefore, nothing happens during function call `XFEM::healMesh()`. The cut marks element 1 for cutting, and it is cut into children elements 3 and 4 during the function call `XFEM::cutMeshWithEFA(...)`. When child element 3 is created, its parent element (1), the correponding cut (1), and its cut subdomain (1) are stored into the current `CEI`, denoted here as $\{3,1,1,1\}$. Similarly, a `CEI` entry $\{4,1,1,2\}$ is stored for child element 4. The system is solved again after the interface has been introduced.

!media media/xfem/healing_recut_2.png
       id=healing_recut_2
       style=width:100%;padding:20px;
       caption=The *healing-and-re-cut* algorithm at time step 2. For demonstration purposes, the `CEI`s are denoted using element IDs and the cut ID, while in the actual implementation pointers are used to store these information.

In the second time step, the system is first solved with updated boundary conditions, during which the signed distance function may be updated. In this case, the interface moves towards the right. During the function call `XFEM::healMesh()`, the cut is removed by deleting element 4. Since child element 3 is now treated as a potential parent element, the corresponding entries in `CEI` are updated to reflect that.

Then, the cut marks elements 3 and 2 for cutting based on the updated signed distance function. During the function call `XFEM::cutMeshWithEFA(...)`, the current `CEI` is first copied into the "old" `CEI`, and four new entries are stored in the current `CEI` as shown in [healing_recut_2]. Whenever a new entry is stored in the current `CEI`, we search in the old `CEI` to check if this new child element was previously healed. A new child element is said to be previously healed if an entry in the old `CEI` has the same parent element ID, the same cut, +AND+ the same cut subdomain ID. In this case, the new `CEI` entry $\{3,3,1,1\}$ matches the old `CEI` entry $\{3,3,1,1\}$, therefore cached material properties on the previously healed child element 3 are copied onto the new child element 3. Similarly, cached material properties on the previously healed child element 4 are copied onto the new child element 4.

On the other hand, the new `CEI` entries $\{5,2,1,1\}$ and $\{6,2,1,2\}$ are not found in the old `CEI` data structure. In this case, material properties of the parent element 2 are copied onto children elements 5 and 6. The system is then solved with the updated interface.

!media media/xfem/healing_recut_3.png
       id=healing_recut_3
       style=width:100%;padding:20px;
       caption=The *healing-and-re-cut* algorithm at time step 3. For demonstration purposes, the `CEI`s are denoted using element IDs and the cut ID, while in the actual implementation pointers are used to store these information.

In the third time step, things are mostly the same as in the second time step, except that the healed element 4 is not being re-cut by the updated signed distance function. In this case, when children elements 3 and 4 are healed, (i.e. the child element 3 is deleted, and the child element 4 is kept), the algorithm queries the new cut subdomain ID on element 4. Since element 4 now has cut subdomain ID of 2, the material properties on the previously healed child element 3 are copied onto element 4.
