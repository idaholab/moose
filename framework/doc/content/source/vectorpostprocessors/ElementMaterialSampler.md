# ElementMaterialSampler

!syntax description /VectorPostprocessors/ElementMaterialSampler

The `ElementMaterialSampler` output to CSV is sorted as follows:

- the first column is the element ID, the last column is the quadrature point, and the columns in between are all the material properties from the requested [`Material`](syntax/Materials/index.md).

- the rows are major (outer) sorted by element ID and minor (inner) sorted by quadrature point. Quadrature points are sorted by increasing order. Element IDs are also sorted by increasing order, with no regards to the user ordering in the `elem_ids` parameter.

!alert note title=Vector names
`ElementMaterialSampler` declares vectors named `elem_ids`, `qp_ids`, and a vector for each property requested with the name of the property.

!alert note
Only scalar-valued (floating point and integer-valued) material properties are supported by the vector postprocessor. Vector-valued material properties are not currently supported, though this addition would not be difficult and would be a welcome contribution.

## Example input syntax

In this example, we request in a `ElementMaterialSampler` the output of the three material properties defined by the `mat` material.

!listing test/tests/vectorpostprocessors/element_material_sampler/basic.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/ElementMaterialSampler

!syntax inputs /VectorPostprocessors/ElementMaterialSampler

!syntax children /VectorPostprocessors/ElementMaterialSampler
