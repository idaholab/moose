# FillBetweenPointVectorsTools Namespace

`FillBetweenPointVectorsTools` contains tools that can be used to generate a triangular element transition layer mesh to connect two given curves (i.e., vectors of points) in the XY plane. It was originally developed for `PeripheralModifyGenerator` of the `Reactor` module. As these tools may also be useful for other applications, they are made available in this namespace. In this document, the algorithm of the tools are described.

## Fundamentals

This tool set was designed to create a mesh for a transition layer. A transition layer accommodates the shape and node placement of two pre-existing boundaries and fills the gap between them with elements. The most important input data needed to generate a transition layer is the node positions of the two boundaries. The generated mesh conforms to these two boundaries and connects the end nodes of each boundary using a straight line, as indicated in [Figure 1](#transition).

!media framework/utils/transition_layer.png
      style=display: block;margin-left:auto;margin-right:auto;width:50%;
      id=transition
      caption=A schematic drawing showing the fundamental functionality of the `FillBetweenPointVectorsTools`

## Single-Layer Transition Layer Meshing

The most straightforward solution is to create a single layer of triangular elements as the transition layer. A triangular element is created by selecting three vertices from the two sets of boundary nodes. One node is selected from one of the two pre-existing boundaries and two nodes are selected from the other boundary. The selection of the nodes should minimize the length of sides connecting the two boundaries. This algorithm is illustrated in [Figure 2](#single_layer).

!media framework/utils/single_layer.png
      style=display: block;margin-left:auto;margin-right:auto;width:50%;
      id=single_layer
      caption=A schematic drawing showing the principle of single-layer transition layer meshing algorithm.

Starting from the first nodes of the two given boundaries, the first side is trivially created by connecting the first nodes of the two boundaries. Then, the two possible options of the next side of the triangle are examined, and the shorter length segment between the two boundaries is selected. This kind of selection is repeated until reaching the other side of the two boundaries.

## Multi-Layer Transition Layer Meshing

In many cases, more than one layer of triangular elements is desired to improve mesh quality. The generation of a transition layer containing multiple sublayers can be done by repeating the single-layer transition layer meshing steps once the nodes of the intermediate sublayers are generated. Thus, the key procedure here is to create those intermediate nodes based on the two given vectors of nodes on the input boundaries. Here, the algorithm to generate the nodes for each sublayer is described from the simplest case to the most generalized scenario.

### Surrogate Node Interpolation Algorithm

Surrogate node interpolation algorithm is the most fundamental method used in this tool set for intermediate node generation. For simplicity, assume a case where all the nodes on each boundary are uniformly distributed. (Namely, the distance between neighboring nodes within a boundary is equal.) Assume that the two boundaries have $M$ nodes (Side 1) and $N$ nodes (Side 2), respectively, and that there are $K$ sublayers of elements in between. From Side 1 to Side 2, using arithmetic progression, the $k$th layer of intermediate nodes have $S=\lceil M+k(N-M)/K \rfloor$ nodes. To get the positions of these nodes, surrogate nodes are first calculated on the two input boundaries using interpolation leveraging MOOSE's [`LinearInterpolation`](framework/src/utils/LinearInterpolation.C) utility.

!listing /LinearInterpolation.h
         start= LinearInterpolationTempl(const std::vector<Real> & X,
         end= LinearInterpolationTempl() : _x(std::vector<Real>()), _y(std::vector<Real>()), _extrap(false) {}

Here, take Side 1 as an example. As mentioned above, Side 1 has $M$ nodes, the coordinates of which are $(x_0,y_0,z_0)$, $(x_1,y_1,z_1)$, ..., $(x_{M-1},y_{M-1},z_{M-1})$. To get interpolated coordinates of the nodes on Side 1, the coordinate parameters $\{x_i\}$ and $\{y_i\}$ will be the dependent variables of interpolation (i.e., $Y$ in the [`LinearInterpolation`](framework/src/utils/LinearInterpolation.C) of MOOSE), while the $X$ was set as {$0$, $1/(M-1)$, $2/(M-1)$,...,$(M-2)/(M-1)$, $1$} (equal intervals). Note that $\{z_i\}$ does not need interpolation as we are working in the XY plane. For an intermediate layer with $S$, $S$ surrogate nodes are created on Side 1 using the aforementioned interpolation data and the following $X$ values {$0$, $1/(S-1)$, $2/(S-1)$,...,$(S-2)/(S-1)$, $1$}. Meanwhile, another $S$ surrogate nodes are created on Side 2 using a similar approach. Finally, the positions of the $S$ intermediate nodes can be calculated by further interpolating the surrogate nodes created on the two boundaries. In [Figure 3](#multi_layer_uniform), an example of applying surrogate node interpolation algorithm to a boundary with 9 uniformly distributed nodes and a boundary with 4 uniformly distributed nodes to generate an intermediate node layer with six nodes is illustrated.

!media framework/utils/multi_layer_uniform.png
      style=display: block;margin-left:auto;margin-right:auto;width:50%;
      id=multi_layer_uniform
      caption=A schematic drawing showing an example of surrogate node interpolation algorithm. Blue and green nodes belong to the original boundaries; yellow nodes are surrogate nodes generated by linear interpolation on the two original boundaries; and orange nodes are the produced intermediate layer nodes calculated by interpolating the surrogate nodes on the two boundaries.

### Weighted Surrogate Nodes

A more general scenario is that the nodes on the two original boundaries are not uniformly distributed. In that case, weights need to be used during the linear interpolation for surrogate node generation. Again, given a boundary (Side 1) with $M$ nodes, {$^1p_0$, $^1p_1$,...,$^1p_{M-2}$, $^1p_{M-1}$}, the distance between the neighboring nodes are {$^1l_1$, $^1l_2$,...,$^1l_{M-2}$,$^1l_{M-1}$}. The total length of Side 1 is $L=\Sigma_{i=1}^{M-1}{l_i}$. This boundary can be mapped to a boundary with uniformly distributed nodes. For the new boundary, each segment has a weight $^1w_i=(M-1)l_i/L$. Surrogated nodes can then be generated on the new boundary using the same approach as mentioned in the previous subsection. After that, using the weights calculated before, the surrogate nodes are derived to weighted surrogate nodes. After repeating these steps on Side 2, the intermediate nodes can be generated. These procedures are visualized in [Figure 4](#weighted_surrogate).

!media framework/utils/weighted_surrogate.png
      style=display: block;margin-left:auto;margin-right:auto;width:80%;
      id=weighted_surrogate
      caption=A schematic drawing showing an example of weighted surrogate node interpolation algorithm used for intermediate nodes generation when non-uniform distributed nodes are involved on the two original boundaries.

### Quadrilateral Element Transition Layer in a Special Case

`FillBetweenPointVectorsTools` is generally designed for meshing with triangular elements because of their flexibility in accommodating complex node distribution. However, if Side 1 and Side 2 boundaries have the same number of nodes, then the transition layer can be meshed using quadrilateral elements straightforwardly. `FillBetweenPointVectorsTools` is equipped with this special quadrilateral meshing capability.

## Applications

In `FillBetweenPointVectorsTools`, the transition layer generation functionality is provided as a method shown as follows:

!listing /FillBetweenPointVectorsTools.h
         start= void fillBetweenPointVectorsGenerator(ReplicatedMesh & mesh,
         end= /**

Here, `mesh` is a reference `ReplicatedMesh` to contain the generated transition layer mesh; `boundary_points_vec_1` and `boundary_points_vec_2` are vectors of nodes for Side 1 and Side 2 boundaries; `num_layers` is the number of element sublayers; `transition_layer_id` is the subdomain ID of the generated transition layer elements; `input_boundary_1_id` and `input_boundary_2_id` are the IDs of the boundaries of the generated transition layer mesh corresponding to the input Sides 1 and 2, respectively; `begin_side_boundary_id` and `end_side_boundary_id` are the IDs of the other two boundaries of the generated transition layer mesh that connect the starting and ending points of the two input Sides; and `type` and `name` are the class type and object name of the mesh generator calling this method for error message generation purpose. If `boundary_points_vec_1` and `boundary_points_vec_2` have the same size, `quad_elem` can be set as `true` so that quadrilateral elements instead of triangular elements are used to construct the transition layer mesh. In addition, `bias_parameter` can be used to control the meshing biasing of the element sublayers. By default, a non-biased sublayer meshing (i.e., equally spaced) is selected by setting `bias_parameter` as 1.0. Any positive `bias_parameter` is used as the manually set biasing factor, while a zero or negative `bias_parameter` activates automatic biasing, where the local node density values on the two input boundaries are used to determine the local biasing factor. If automatic biasing is selected, `sigma` is used as the Gaussian parameter to perform Gaussian blurring to smoothen the local node density to enhance robustness of the algorithm.

!media framework/utils/transition_layer_tools_examples.png
      style=display: block;margin-left:auto;margin-right:auto;width:80%;
      id=examples
      caption=Some representative meshes generated by `FillBetweenPointVectorsTools`: (left) a transition layer mesh defined by two oppositely oriented arcs; (middle) a transition layer mesh defined by one arc and a complex curve; (right) a half-circle mesh.

One application of this tool is to generate a mesh with two curves and two straight lines as its external boundaries. As shown in [Figure 5](#examples), a series of simple and complex shapes can be meshed. Users can leverage [`FillBetweenPointVectorsGenerator`](/FillBetweenPointVectorsGenerator.md) and [`FillBetweenSidesetsGenerator`](/FillBetweenSidesetsGenerator.md) as testing tools.
