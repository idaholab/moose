# VectorOfPostprocessors

!syntax description /VectorPostprocessors/VectorOfPostprocessors

This vector postprocessor is mainly used for two purposes:

- to output only a selection of postprocessors to a desired CSV file, similarly to a `CSV` output. The CSV output of the `VectorOfPostprocessors` is then column-ordered by the order of postprocessor specified in the `postprocessors` parameter.

- to combine postprocessors to match the expected vector postprocessor input of a kernel, a user object, etc

!alert note title=Vector names / CSV output column names
`VectorOfPostprocessors` declares a vector with its own name. The full reporter name ends up being `<vpp_object_name>/<vpp_object_name>`.

## Example input syntax

In this example, the `min` and `max` postprocessors are combined in the `min_max` `VectorOfPostprocessors`.

!listing test/tests/vectorpostprocessors/vector_of_postprocessors/vector_of_postprocessors.i block=VectorPostprocessors

!syntax parameters /VectorPostprocessors/VectorOfPostprocessors

!syntax inputs /VectorPostprocessors/VectorOfPostprocessors

!syntax children /VectorPostprocessors/VectorOfPostprocessors
