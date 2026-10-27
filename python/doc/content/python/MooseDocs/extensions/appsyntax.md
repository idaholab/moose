# AppSyntax Extension

The AppSyntax Extension defines the `!syntax` markdown [command](/command.md), which contains various
sub-commands for creating MOOSE specific links and tables within your markdown, each are explained
below. Note, the examples below refer to documentation associated with Kernels and/or the Diffusion
Kernel. This should be replaced by the syntax for the system or object being documented. The
configuration options for the extension itself are listed in [appsyntax-config].

!devel settings module=MooseDocs.extensions.appsyntax
                object=AppSyntaxExtension
                id=appsyntax-config
                caption=Configuration options for the AppSyntax extension.

## Class Description (`!syntax description`)

The `description` sub-command adds a paragraph with the content of InputParameters object class
descriptions, which is added in the MOOSE application within the `validParams` method. The available
settings are included in [appsyntax-description-settings].

!devel settings module=MooseDocs.extensions.appsyntax
                id=appsyntax-description-settings
                object=SyntaxDescriptionCommand
                caption=Command settings for the `!syntax description` command.

For example, the FileMesh object includes a `validParams` function as shown in
[file-mesh-valid-params]. Notice, that the `addClassDescription` method includes a short
description of the object. To display this text the `!syntax description` command is used followed by
the MOOSE input file syntax for the object as in [appsyntax-description].

!devel! example id=appsyntax-description caption=Example use of the `!syntax description` command.
!syntax description /Mesh/FileMesh
!devel-end!

!listing framework/src/mesh/FileMesh.C start=template
                                       end=}
                                       include-end=True
                                       id=file-mesh-valid-params
                                       caption=The validParams function from the FileMesh object.


## Object Parameters (`!syntax parameters`)

The `parameters` sub-command provides a means for displaying the default input file syntax for an
object as shown in [appsyntax-parameters-example]. The available settings for the parameters
command are shown in [appsyntax-parameters-settings].

!devel settings module=MooseDocs.extensions.appsyntax
                object=SyntaxParametersCommand
                id=appsyntax-parameters-settings
                caption=Command settings for the `!syntax parameters` command.


!devel! example id=appsyntax-parameters-example caption=Example use of the `!syntax parameters` command.
!syntax parameters /Dampers/ConstantDamper
!devel-end!

## Input Files (`!syntax inputs`) id=inputs

In many cases it is useful to know where in the examples, tutorials, or tests an object is utilized
in an input file. Therefore, the `inputs` sub-command is defined. [appsyntax-inputs-example] lists
all the input files that include the ConstantDamper object and [appsyntax-inputs-settings]
provides the available settings for the `inputs` command.

!devel settings module=MooseDocs.extensions.appsyntax
                object=SyntaxInputsCommand
                id=appsyntax-inputs-settings
                caption=Command settings for the `!syntax inputs` command.

!devel! example id=appsyntax-inputs-example caption=Example use of the `!syntax inputs` command.
!syntax inputs /Dampers/ConstantDamper
!devel-end!


## Child Objects (`!syntax children`)

MOOSE is designed on the idea of
[inheritance](https://en.wikipedia.org/wiki/Inheritance_(object-oriented_programming%29), as such it is
often useful to understand which classes inherit from another. The `children` sub-command creates a
list of all child classes of the given object, as shown in [appsyntax-children-example]. The
available settings for the `children` command are provided below.

!devel! example id=appsyntax-children-example caption=Example use of the `!syntax children` command.
!syntax children /Kernels/Diffusion
!devel-end!

!devel settings module=MooseDocs.extensions.appsyntax
                object=SyntaxChildrenCommand
                id=appsyntax-children-settings
                caption=Command settings for the `!syntax children` command.

## Object Parameter (`!param`) id=param

While the `!syntax parameters` command (see [appsyntax-parameters-example]) displays an entire
input parameters table, the `param` command displays a single input file parameter inline within a
sentence, which is useful for referring to a specific parameter within descriptive text. Unlike the
other commands on this page, `param` is invoked using the markdown link syntax rather than the
`!command subcommand` form, with the object syntax and parameter name (separated by a slash)
supplied as the link target, as in [appsyntax-param-example].

!devel! example id=appsyntax-param-example caption=Example use of the `!param` command.
The `variable` parameter, [!param](/Kernels/Diffusion/variable), of the Diffusion kernel is required.
!devel-end!

The resulting link opens a modal window containing the same information (type, description, default,
etc.) that would appear for that parameter within a `!syntax parameters` table.

If the referenced object syntax or parameter name cannot be found, an error is raised that lists the
closest matching parameter names, to aid in catching typos.

If the page containing the `!param` command is associated with content marked `external` (see
[MooseDocs/config.md#optional-dependencies]), the parameter information is not available because the
application supplying that syntax was not built. In this case, `!param` skips creating the modal
link, logs a warning, and instead renders the plain object syntax and parameter name as placeholder
text so that the surrounding sentence is not left with a gap.

## Actions, Objects, and Systems (`!syntax list`)

MOOSE is based on systems (e.g., Kernels), where each system contains a set of objects, actions,
and subsystems that can be used within an input file block. It is desirable to list the available
object for a given system, which is the reason behind the `!syntax list` command.  The available
settings for this command are included in [appsyntax-list-settings].

!devel settings module=MooseDocs.extensions.appsyntax
                object=SyntaxListCommand
                id=appsyntax-list-settings
                caption=Command settings for the `!syntax list` command.


!devel! example id=appsyntax-list-example caption=Example use of the `!syntax list` command.
!syntax list /Adaptivity
!devel-end!

## Complete Syntax List

```
!syntax complete level=3
```
