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
  of the component and `block_name` is the original boundary name.

!syntax parameters /Components/FileMeshComponent

!syntax inputs /Components/FileMeshComponent

!syntax children /Components/FileMeshComponent
