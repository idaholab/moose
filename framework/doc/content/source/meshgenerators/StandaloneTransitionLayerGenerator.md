# StandaloneTransitionLayerGenerator

!syntax description /Mesh/StandaloneTransitionLayerGenerator

## Overview

The `StandaloneTransitionLayerGenerator` class uses the fundamental functionalities of [`TransitionLayerTools`](/TransitionLayerTools.md). Therefore, this class provides a testing tool for `TransitionLayerTools` as well as a generalized platform for users to create meshes using the tool set. Users are required to provide the three major inputs needed to use `TransitionLayerTools`:

- [!param](/Mesh/StandaloneTransitionLayerGenerator/positions_vector_1) and [!param](/Mesh/StandaloneTransitionLayerGenerator/positions_vector_2): the vectors of points on the two boundaries (i.e., Side 1 and Side 2).
- [!param](/Mesh/StandaloneTransitionLayerGenerator/num_layers): number of element sublayers.

Aside from these fundamental input parameters, users can also assign block and the external boundary IDs through [!param](/Mesh/StandaloneTransitionLayerGenerator/block_id) and [!param](/Mesh/StandaloneTransitionLayerGenerator/input_boundary_1_id)/[!param](/Mesh/StandaloneTransitionLayerGenerator/input_boundary_2_id)/[!param](/Mesh/StandaloneTransitionLayerGenerator/begin_side_boundary_id)/[!param](/Mesh/StandaloneTransitionLayerGenerator/end_side_boundary_id).

!media framework/meshgenerators/transition_layer_examples.png
      style=display: block;margin-left:auto;margin-right:auto;width:40%;
      id=transition
      caption=A schematic drawing showing different transition layer meshes generated between two arc boundaries: (left to right) very fine mesh, fine mesh, and coarse mesh; (top to bottom) uniformly distributed nodes, slightly biased nodes, and heavily biased nodes.

In general, `StandaloneTransitionLayerGenerator` handles many different scenarios. As shown in [Figure 1](#transition), non-uniformly distributed boundary nodes (i.e., biased) may be input. The mesh generator does have some limitations. For example, the two input curves cannot intersect each other; and the interpolated nodes should not lead to flipped elements or overlapped elements. Due to the complexity of geometry, the mesh generator may not produce an error message in all the problematic cases. Users should cautiously examine the generated mesh by setting [!param](/Mesh/StandaloneTransitionLayerGenerator/show_info) as `true` and by running a simple diffusion problem.

The spacings of element sublayers can be biased by setting [!param](/Mesh/StandaloneTransitionLayerGenerator/bias_parameter). Any positive [!param](/Mesh/StandaloneTransitionLayerGenerator/bias_parameter) is directly used as the fixed mesh biasing factor with the default value 1.0 for non-bias. By setting [!param](/Mesh/StandaloneTransitionLayerGenerator/bias_parameter) as 0.0, automatic biasing will be used, where the local node density values on the two input boundaries are used to determine the local biasing factor (see [Figure 2](#bias) as an example). In that case, Gaussian blurring is used to smoothen the local node density to enhance stability of the algorithm, which can be tuned through [!param](/Mesh/StandaloneTransitionLayerGenerator/gaussian_sigma).

!media framework/meshgenerators/biased_transition_layer.png
      style=display: block;margin-left:auto;margin-right:auto;width:40%;
      id=bias
      caption=A schematic drawing showing different biasing option for sublayers: (left) non-bias; (middle) fixed biasing factor = 0.8; (right) automatic biased based on boundary nodes.


In some special cases, when [!param](/Mesh/StandaloneTransitionLayerGenerator/positions_vector_1) and [!param](/Mesh/StandaloneTransitionLayerGenerator/positions_vector_2) have the same length, users can set [!param](/Mesh/StandaloneTransitionLayerGenerator/use_quad_elements) as true to construct the transition layer mesh using quadrilateral elements (see [Figure 3](#quad) as an example).

!media framework/meshgenerators/quad_transition.png
      style=display: block;margin-left:auto;margin-right:auto;width:40%;
      id=quad
      caption=A schematic drawing showing a transition layer meshed by quadrilateral elements.

## Example Syntax

!listing test/tests/meshgenerators/standalone_transition_layer_generator/bow.i block=Mesh

!syntax parameters /Mesh/StandaloneTransitionLayerGenerator

!syntax inputs /Mesh/StandaloneTransitionLayerGenerator

!syntax children /Mesh/StandaloneTransitionLayerGenerator
