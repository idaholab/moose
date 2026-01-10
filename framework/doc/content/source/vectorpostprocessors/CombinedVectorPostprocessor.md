# CombinedVectorPostprocessor

!syntax description /VectorPostprocessors/CombinedVectorPostprocessor

The vectors in the combined vectorpostprocessor are renamed with the name of the vectorpostprocessor,
followed by an underscore, followed by the name of the vector in the original vectorpostprocessor.

In the CSV output, if the vectors are not of the same length, the number of rows is determined by the longer
vector, and the rest of the other vectors are filled with the value chosen in the
[!param](/VectorPostprocessors/CombinedVectorPostprocessor/vector_filler_value) parameter.

!syntax parameters /VectorPostprocessors/CombinedVectorPostprocessor

!syntax inputs /VectorPostprocessors/CombinedVectorPostprocessor

!syntax children /VectorPostprocessors/CombinedVectorPostprocessor
