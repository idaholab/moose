# AppSyntaxExtension

The AppSyntaxExtension defines the `!syntax` markdown keyword, which contains various sub-commands
for creating MOOSE specific links and tables within your markdown,
each are explained below. Note, the examples below refer to documentation associated with Kernels
and/or the Diffusion Kernel. This should be replaced by the syntax for the system or object being
documented. The configuration options for the extension itself are listed in \ref{AppSyntaxExtension}.

!extension AppSyntaxExtension

## Class Description
#### `!syntax description`

The `description` sub-command adds an html paragraph with the content of InputParameters object
class descriptions, which is added in the MOOSE application within the `validParams` method.

For example, the [FileMesh] object includes a `validParams` function
as shown in \ref{file-mesh-valid-params}. Notice, that the `addClassDescription` method includes a short description of the object. To display this text the `!syntax description` command is used followed
by the [MOOSE] input file syntax for the object as follows.

```markdown
!syntax description /Mesh/FileMesh style=color:green
```

!syntax description /Mesh/FileMesh style=color:green

!listing framework/src/mesh/FileMesh.C start=template<> end=FileMesh::FileMesh id=file-mesh-valid-params caption=The validParams function from the [FileMesh] object.

!extension-settings moose_syntax_description caption=Command settings for `!syntax description` command.

## Object Parameters
#### `!syntax parameters`

The `parameters` sub-command provides a means for displaying the default input file syntax for an
object. For example, considering the [FileMesh] object, the complete list of input syntax can be
provided using the following markdown command, the results of which are shown on the [Input Parameters](framework/FileMesh.md#input-parameters) section of the [FileMesh] page.

```markdown
!syntax parameters /Mesh/FileMesh
```

!extension-settings moose_syntax_parameters caption=Command settings for `!syntax parameters` command.

## Input Files
#### `!syntax inputs`

In many cases it is useful to know where in the examples, tutorials, or tests an object is utilized
in an input file. Therefore, `inputs` sub-command is defined.

For example, the following markdown is included on the [Diffusion] page to create the
[Input Files](framework/Diffusion.md#input-files)  sections, respectively.


```markdown
!syntax inputs /Kernels/Diffusion
```

!extension-settings moose_input_list caption=Command settings for `!syntax inputs` commands.


## Child Objects
#### `!syntax children`
[MOOSE] is designed on the idea of [inheritance](https://en.wikipedia.org/wiki/Inheritance_(object-oriented_programming), as such it
is often useful to understand which classes inherit from another. The `children` sub-command creates
a list of all child classes of the given object. Again, the Diffusion object is used, the following
markdown creates the content in the [Child Objects](framework/Diffusion.md#child-objects) section
of the [Diffusion](framework/Diffusion.md) page.

```markdown
!syntax children /Kernels/Diffusion
```

!extension-settings moose_child_list caption=Command settings for `!syntax children` commands.

## Sub-Objects
#### `!syntax objects`

[MOOSE] is based on systems (e.g., [Kernels]), where each system contains a set of objects that
can be used within an input file block. It is desirable to list the available object for a given
system, which is the reason behind the `objects` sub-command.

The markdown below creates a list of objects available for the [Markers] system.

```markdown
!syntax objects /Adaptivity/Markers
```

!extension-settings moose_object_list caption=Command settings for `!syntax objects` commands.


## Actions
#### `!syntax actions`

[MOOSE] includes an "actions" system for defining custom input file syntax and multiple actions
may be associated with a single system. For example, the [Markers] system contains the [AddElementalFieldAction](Markers/framework/AddElementalFieldAction.md) and the [AddMarkerAction](framework/AddMarkerAction.md). To list these in
similar fashion as object, the `actions` sub-command is used. Again, for [Markers] the following
may be used.

```markdown
!syntax actions /Adaptivity/Markers
```

!extension-settings moose_action_list caption=Command settings for `!syntax actions` commands.

## Sub-Systems
#### `!syntax subsystems`
For [MOOSE] syntax that contain additional sub-blocks (e.g., [Adaptivity]) a list of the
available sub-systems for the given syntax can be created using this command. For example, the [Available Sub-Systems](systems/Adaptivity/index.md#available-sub-systems) sections was generated
using the following.

```markdown
!syntax subsystems /Adaptivity
```

!extension-settings moose_subsystem_list caption=Command settings for `!syntax subsystems` command.


## Complete System List
To present a complete view of all the available syntax and objects the `!syntax complete` command
exists, this command is used to create pages such as the [MOOSE
Systems](documentation/systems/index.md) pages.

!extension-settings moose_complete_syntax caption=Command settings for `!syntax complete` command.

[FileMesh]: /framework/FileMesh.md
[Diffusion]: /framework/Diffusion.md
