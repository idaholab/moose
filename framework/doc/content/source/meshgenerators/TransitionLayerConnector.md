# TransitionLayerConnector

!syntax description /Mesh/TransitionLayerConnector

## Overview

The `TransitionLayerConnector` offers similar functionality to [`TransitionLayerGenerator`](/TransitionLayerGenerator.md) by leveraging [`TransitionLayerTools`](/TransitionLayerTools.md). Instead of manually inputting the two boundaries [!param](/Mesh/TransitionLayerGenerator/positions_vector_1) and [!param](/Mesh/TransitionLayerGenerator/positions_vector_2), The `TransitionLayerConnector` directly takes boundary information ([!param](/Mesh/TransitionLayerConnector/boundary_1) and [!param](/Mesh/TransitionLayerConnector/boundary_2)) of two input meshes, [!param](/Mesh/TransitionLayerConnector/input_mesh_1) and [!param](/Mesh/TransitionLayerConnector/input_mesh_2). The input meshes can be translated using [!param](/Mesh/TransitionLayerConnector/mesh_1_shift) and [!param](/Mesh/TransitionLayerConnector/mesh_2_shift). The generated transition layer mesh can be output as a standalone mesh or a stitched mesh with the input meshes, depending on [!param](/Mesh/TransitionLayerConnector/keep_inputs).

!media framework/meshgenerators/transition_layer_stitched.png
      style=display: block;margin-left:auto;margin-right:auto;width:80%;
      id=example_tlc
      caption=A typical example of using `TransitionLayerConnector` to connect two square meshes together.

All the other meshing options are the same as [`TransitionLayerGenerator`](/TransitionLayerGenerator.md).

## Example Syntax

!listing test/tests/meshgenerators/transition_layer_connector/squares.i block=Mesh/tlc

!syntax parameters /Mesh/TransitionLayerConnector

!syntax inputs /Mesh/TransitionLayerConnector

!syntax children /Mesh/TransitionLayerConnector
