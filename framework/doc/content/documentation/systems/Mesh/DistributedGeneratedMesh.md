# DistributedGeneratedMesh

!syntax description /Mesh/DistributedGeneratedMesh

## Description

Similar to [/GeneratedMesh.md] - builds lines, rectangles and rectangular prisms.  It differs though in the way the mesh is constructed in parallel.  While `GeneratedMesh` creates a full copy of the mesh on every processor - `DistributedGeneratedMesh` only creates the elements / nodes each processor is assigned.  This makes it _much_ faster in parallel and much leaner in memory.

!alert note
Be aware: `DistributedGeneratedMesh` turns on `parallel_type = distributed`.  So make sure that everything you're using in your problem works with that before using this!

### More Information

`DistributedGeneratedMesh` works by first creating a "dual graph" of the element connectivity - before ever building an elements.  It then uses METIS to partition that graph - assigning elements to processors.  Then, each processor can read the partition map and build only the elements that need to be on that processor.  Final steps include adding in "ghosted" elements and making sure that boundary conditions are right.

## Developer Information

If you're going to enhance `DistributedGeneratedMesh` the structure is based off of template specialization.  To make it build new element types you need to specialize a number of templated methods.  These can be found in `DistributedGeneratedMesh.C`:

```C++
dof_id_type
elem_id(const dof_id_type /*nx*/,
        const dof_id_type /*ny*/,
        const dof_id_type /*i*/,
        const dof_id_type /*j*/,
        const dof_id_type /*k*/);

dof_id_type
num_neighbors(const dof_id_type /*nx*/,
              const dof_id_type /*ny*/,
              const dof_id_type /*nz*/,
              const dof_id_type /*i*/,
              const dof_id_type /*j*/,
              const dof_id_type /*k*/);

void
get_neighbors(const dof_id_type /*nx*/,
              const dof_id_type /*ny*/,
              const dof_id_type /*nz*/,
              const dof_id_type /*i*/,
              const dof_id_type /*j*/,
              const dof_id_type /*k*/,
              std::vector<dof_id_type> & /*neighbors*/);

inline dof_id_type
node_id(const ElemType /*type*/,
        const dof_id_type /*nx*/,
        const dof_id_type /*ny*/,
        const dof_id_type /*i*/,
        const dof_id_type /*j*/,
        const dof_id_type /*k*/);

Node *
add_point(const dof_id_type /*nx*/,
          const dof_id_type /*ny*/,
          const dof_id_type /*nz*/,
          const dof_id_type /*i*/,
          const dof_id_type /*j*/,
          const dof_id_type /*k*/,
          const ElemType /*type*/,
          MeshBase & /*mesh*/);

void
add_element(const dof_id_type /*nx*/,
            const dof_id_type /*ny*/,
            const dof_id_type /*nz*/,
            const dof_id_type /*i*/,
            const dof_id_type /*j*/,
            const dof_id_type /*k*/,
            const dof_id_type /*elem_id*/,
            const processor_id_type /*pid*/,
            const ElemType /*type*/,
            MeshBase & /*mesh*/,
            bool /*verbose*/);

void
get_indices(const dof_id_type /*nx*/,
            const dof_id_type /*ny*/,
            const dof_id_type /*elem_id*/,
            dof_id_type & /*i*/,
            dof_id_type & /*j*/,
            dof_id_type & /*k*/);

void
get_ghost_neighbors(const dof_id_type /*nx*/,
                    const dof_id_type /*ny*/,
                    const dof_id_type /*nz*/,
                    const MeshBase & /*mesh*/,
                    std::set<dof_id_type> & /*ghost_elems*/);

void
set_boundary_names(BoundaryInfo & /*boundary_info*/);
```

## Example Syntax

!listing test/tests/functions/image_function/image_mesh_3d.i block=Mesh

!syntax parameters /Mesh/ImageMesh

!syntax inputs /Mesh/ImageMesh

!syntax children /Mesh/ImageMesh
