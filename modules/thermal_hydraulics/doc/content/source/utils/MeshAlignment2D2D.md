# MeshAlignment2D2D

This class inherits from [MeshAlignmentOneToMany.md] and builds a mapping between
elements/faces between multiple 2D boundaries. The first entry in the `boundary_infos`
argument of the

```
void initialize(boundary_infos, axis_point, axis_direction)
```

method is taken to be the "primary" boundary. Each element on this boundary is
mapped to the corresponding element in each of the other boundaries. The coupled
secondary element IDs for a primary element ID `primary_elem_id` are obtained via

```
const std::vector<dof_id_type> & getCoupledSecondaryElemIDs(primary_elem_id)
```

The boundaries must be *aligned*; the axis is defined by `axis_point` and `axis_direction`,
and the axial coordinates of the primary side element and the coupled secondary
elements must be exactly the same.
