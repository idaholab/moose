# AppSyntaxExtension

A set of special keywords exist for creating MOOSE specific links and tables within your markdown,
each are explained below. Note, the examples below refer to documentation associated with Kernels
and/or the Diffusion Kernel. This should be replaced by the syntax for the system or object being
documented.

!extension AppSyntaxExtension

## Class Description

The `!description` command adds an html paragraph with the content of InputParameters object
class descriptions, which is added in the MOOSE application within the `validParams` method.

For example, the [FileMesh] object includes a `validParams` function
as shown in \ref{file-mesh-valid-params}. Notice, that the `addClassDescription` method includes a short description of the object. To display this text the `!description` command is used followed
by the [MOOSE] input file syntax for the object as follows.

```markdown
!description /Mesh/FileMesh style=color:green
```

!description /Mesh/FileMesh style=color:green

!listing framework/src/mesh/FileMesh.C start=template<> end=FileMesh::FileMesh id=file-mesh-valid-params caption=The validParams function from the [FileMesh] object.

!extension-settings moose_description caption=Command settings for `!description` command.

## Object Parameters

The `!parameters` command provides a means for displaying the default input file syntax for an
object. For example, considering the [FileMesh] object, the complete list of input syntax can be
provided using the following markdown command, the results of which are shown on the [Input Parameters](framework/FileMesh.md#input-parameters) section of the [FileMesh] page.

```markdown
!parameters /Mesh/FileMesh
```

!extension-settings moose_parameters caption=Command settings for `!parameters` command.

## Input Files and Child Objects

In many cases it is useful to know where in the examples, tutorials, or tests an object is utilized
in an input file as well as what other objects may inherit from an object. Therefore, two commands
are provided to create these lists: `!inputfiles` and `!childobjects`, respectively.

For example, the following markdown is included on the [Diffusion] page, these commands create the
[Input Files](framework/Diffusion.md#input-files) and [Child Objects](framework/Diffusion.md#child-objects) sections, respectively.


```markdown
!inputfiles /Mesh/FileMesh

!childobjects /Mesh/FileMesh
```

!extension-settings moose_object_syntax caption=Command settings for `!inputfiles` and `!childobjects` commands.

## Sub-Objects and Sub-Systems Lists

Similarly, to the how an object is used it is also desirable to list the available objects for a
certain system, this can be done using the `!subobjects` and `!subsystems` commands.`

* `!subobjects`<br>
For input file syntax that can take arbitrary object blocks (e.g., [Mesh]) a list of the available
objects for the given syntax can be created using this command. For example, the [Example Syntax and Mesh Objects](/Mesh/index.md#example-syntax-and-mesh-objects) sections was generated
using the following.

```markdown
!subobjects /Mesh title=None
```

* `!subsystems`<br>
For input file syntax that contain additional sub-blocks (e.g., [Adaptivity]) a list of the
available sub-sytems for the given syntax can be created using this command. For example, the [Available Sub-Systems](systems/Adaptivity/index.md#available-sub-systems) sections was generated
using the following.

```markdown
!subsystems /Adaptivity
```

## Complete System Lists

To present a complete view of all the available syntax and objects the `!systems` command exists,
this command is used to create pages such as the [MOOSE Systems](documentation/systems/index.md) pages.

!extension-settings moose_system_list caption=Command settings for `!systems` command.

[FileMesh]: /framework/FileMesh.md
[Diffusion]: /framework/Diffusion.md
