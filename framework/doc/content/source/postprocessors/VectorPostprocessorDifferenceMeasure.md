# VectorPostprocessorDifference

!syntax description /Postprocessors/VectorPostprocessorDifference

This post-processor is used to compare two vector post-processor vectors that
have the same length. It returns a measure of the distance between the two
vectors.
There are two different options for the parameter `difference_type`. Denoting the first
vector as `a` and the second as `b`, these options are as follows:

| Value                | Operation               
|----------------------|--------------------
| `difference`         | `$\sum(a_i - b_i)$`        
| `L2`                 | `$\sqrt(\sum(a_i-b_i)^2)$  

!syntax parameters /Postprocessors/VectorPostprocessorDifference

!syntax inputs /Postprocessors/VectorPostprocessorDifference

!syntax children /Postprocessors/VectorPostprocessorDifference

!bibtex bibliography
