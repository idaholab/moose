# BatchMeshGeneratorAction

## Overview

This Action is capable of batch generating a series of meshes using a single type of mesh generator with variation in one or multiple input parameters.

The type of mesh generator to be used for batch generation is provided by [!param](/Mesh/BatchMeshGeneratorAction/mesh_generator_name). To batch generate meshes using the selected type of mesh generator, users need to provide both `fixed` input parameters, which keep constant throughout the batch generator, and `batch` input parameters, which vary among generated meshes. For both `fixed` anf `batch` input parameters, they are either be scalar or vector type.

## Specifying Input Parameters

### Fixed Input Parameters

Both `scalar` and `vector` types of fixed input parameters are supported by this Action. 

Names of multiple fixed `scalar` input parameters can be defined by [!param](/Mesh/BatchMeshGeneratorAction/fixed_scalar_input_param_names), with their corresponding types defined by [!param](/Mesh/BatchMeshGeneratorAction/fixed_scalar_input_param_types). In the meantime, the values of these fixed `scalar` input parameters need to be provided through [!param](/Mesh/BatchMeshGeneratorAction/fixed_scalar_input_param_values).

On the other hand, names and the corresponding types of multiple fixed `vector` input parameters can be specified by [!param](/Mesh/BatchMeshGeneratorAction/fixed_vector_input_param_names) and [!param](/Mesh/BatchMeshGeneratorAction/fixed_vector_input_param_types), respectively. The values of these fixed `vector` input parameters are provided through [!param](/Mesh/BatchMeshGeneratorAction/fixed_vector_input_param_values).

### Batch Input Parameters

Likewise, both `scalar` and `vector` types of batch input parameters are supported by this Action.

Similar to the `scalar` and `vector` types of fixed input parameters, the names, types, and values of the batch `scalar` and `vector` input parameters can be specified using [!param](/Mesh/BatchMeshGeneratorAction/batch_scalar_input_param_names), [!param](/Mesh/BatchMeshGeneratorAction/batch_scalar_input_param_types), [!param](/Mesh/BatchMeshGeneratorAction/batch_scalar_input_params); as well as [!param](/Mesh/BatchMeshGeneratorAction/batch_vector_input_param_names), [!param](/Mesh/BatchMeshGeneratorAction/batch_vector_input_param_types), [!param](/Mesh/BatchMeshGeneratorAction/batch_vector_input_params), respectively. Note that for the batch input parameters, because variation is needed, the dimensions of [!param](/Mesh/BatchMeshGeneratorAction/batch_scalar_input_params) and [!param](/Mesh/BatchMeshGeneratorAction/batch_vector_input_params) are higher than their fixed counterparts by 1.

### Types of Input Parameters

The supported input parameter types are summarized in [supported_types] for both `scalar` and `vector` types.

!table id=supported_types caption=Summary of the symbols used for supported types of input parameters.
| C++ Vector Data Type | C++ Scalar Data Type | Syntax |
| - | - | - |
| `vector<Real>` | `Real` | `REAL` |
| `vector<short>` | `short` | `SHORT` |
| `vector<unsigned short>` | `unsigned short` | `USHORT` |
| `vector<int>` | `int` | `INT` |
| `vector<unsigned int>` | `unsigned int` | `UINT` |
| `vector<string>` | `string` | `STRING` |
| `MultiMooseEnum` | `MooseEnum` | `ENUM` |
| `vector<bool>` | `bool` | `BOOL` |

## Multiple Batch Input Parameter Processing Methods

If more than one batch input parameters (either `scalar` or `vector`) are provided, two methods can be selected using [!param](/Mesh/BatchMeshGeneratorAction/multi_batch_params_method). 

The default method is `cartesian_product`, which batch generates mesh generators based on the Cartessian product of all the batch input parameters. For example, if `m` scalar input parameters are specified using [!param](/Mesh/BatchMeshGeneratorAction/batch_scalar_input_param_names), with the corresponding [!param](/Mesh/BatchMeshGeneratorAction/batch_scalar_input_params) containing $N_{s,1}$, $N_{s,2}$,..., and $N_{s,m}$ variations, respectively. Meanwhile, if `n` vector input parameters are specified using [!param](/Mesh/BatchMeshGeneratorAction/batch_vector_input_param_names), with the corresponding [!param](/Mesh/BatchMeshGeneratorAction/batch_vector_input_params) containing $N_{v,1}$, $N_{v,2}$,..., and $N_{v,n}$ variations. Using the Cartesian product approach leads to a total of $\prod_{i=0}^{m} N_{s,i}\prod_{j=0}^{n} N_{v,j}$ mesh generators.

On the other hand, an alternative method is `corresponding`, which requires the numbers of variations for all [!param](/Mesh/BatchMeshGeneratorAction/batch_scalar_input_param_names) and [!param](/Mesh/BatchMeshGeneratorAction/batch_vector_input_param_names) to be the same (e.g., $N$). In that case, only $N$ mesh generators are created.

!syntax parameters /Mesh/BatchMeshGeneratorAction
