# ParameterMeshFunction

!syntax description /Functions/ParameterMeshFunction

## Overview

This function is similar to [NearestReporterCoordinatesFunction.md], except it uses an inputted exodus mesh ([!param](/Functions/ParameterMeshFunction/exodus_mesh)) to represent the parameter field and uses finite-element shape functions (defined with [!param](/Functions/ParameterMeshFunction/family) and [!param](/Functions/ParameterMeshFunction/order)) to perform spatial interpolation. This function also interpolates in time using [!param](/Functions/ParameterMeshFunction/time_name), which can be a [vector-postprocessor](VectorPostprocessors/index.md) or a vector [reporter](Reporters/index.md). The parameter data is specified using [!param](/Functions/ParameterMeshFunction/parameter_name), which can be a [vector-postprocessor](VectorPostprocessors/index.md) or a vector [reporter](Reporters/index.md). [!ref](tab:fe_types) shows common interpolation types for the parameters.

!table id=tab:fe_types caption=Common interpolation type with associated finite-element function
| Interpolation Type | Family | Order |
| :- | - | - |
| Piecewise constant | `MONOMIAL` | `CONSTANT` |
| Linear | `LAGRANGE` | `FIRST` |
| Quadratic | `LAGRANGE` | `SECOND` |

The
[!param](/Functions/ParameterMeshFunction/project_points) allows for parameter mesh to be an any linear element mesh that doesn't fully encompass the simulation mesh. A point not encompassed by the parameter mesh will be projected onto the nearest mesh element on the parameter mesh to determine the function value. This method will not work with quad4 that are not coplanar, including other volume elements that may not have coplanar sides.

When using point projection, the [!param](/Functions/ParameterMeshFunction/kdtree_candidates) parameter controls the efficiency of the closest point search algorithm. This parameter determines how many nearest mesh nodes are considered when finding the optimal element for projection. The default value of 5 is sufficient for most cases and provides a good balance between accuracy and performance. Users can increase this value (e.g., to 10-20) for complex geometries where higher accuracy is needed, though values above 10 typically provide diminishing returns while increasing computational cost.

!alert warning
The mesh created +must+ be replicated. Ensure this by having `Mesh/parallel_type=REPLICATED` when creating the mesh.

## Example Input File Syntax

First step is to define a mesh for the parameters, which is most easily done by creating a separate input file. The following creates a two-by-two mesh and sets an auxiliary variable to $x(x-1)y(y-1)$, which is then outputted using [NodalValueSampler.md].

!listing parameter_mesh/create_mesh.i

Running this input will create a `create_mesh_out.e` exodus file and `create_mesh_out_param_vec_0001.csv` CSV file. The CSV file is read by a [CSVReaderVectorPostprocessor.md] to create a vector-postprocessor of the parameter data. A ParameterMeshFunction then reads in the exodus file and retrieves the vector for the interpolation.

!listing parameter_mesh/parameter_mesh.i block=VectorPostprocessors Functions

!syntax parameters /Functions/ParameterMeshFunction

!syntax inputs /Functions/ParameterMeshFunction

!syntax children /Functions/ParameterMeshFunction
