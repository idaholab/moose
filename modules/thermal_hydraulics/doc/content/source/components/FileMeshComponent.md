# FileMeshComponent

This component loads a mesh from an ExodusII file. This class can be used directly
in the input file, which will load a mesh into a component without adding any
physics, or it can be used as a base class for other components that do add
physics.

## Usage

The parameter [!param](/Components/FileMeshComponent/file) is used to specify the
name of the ExodusII file containing a mesh. When loading this mesh into the
component, the component name is prepended to the block and boundary names:

- Blocks become `component_name:block_name`, where `component_name` is the name
  of the component and `block_name` is the original block name.
- Boundaries become `component_name:boundary_name`, where `component_name` is the name
  of the component and `boundary_name` is the original boundary name.

The parameter [!param](/Components/FileMeshComponent/position) specifies a
translation vector $\mathbf{p}$ for the points in the mesh file; the node positions for this component
are computed as

!equation
\mathbf{r} = \mathbf{r}_\text{file} + \mathbf{p} \,,

where $\mathbf{r}_\text{file}$ is a point in the loaded mesh file.

!syntax parameters /Components/FileMeshComponent

!syntax inputs /Components/FileMeshComponent

!syntax children /Components/FileMeshComponent
