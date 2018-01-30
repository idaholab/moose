# Listings Extension

It is possible to include complete or partial C++ or input files from the local MOOSE repository.
The `!listing` command allows for numbered captions to be applied, the [Numbered Floats](extensions/numbered_floats.md) provides additional details.

## Including Code
The ListingExtension has the capability to include text from arbitrary files, such as source code
files. There is a wide range of settings that are available to specialize how the code is imported.
The complete list of available of settings are provided in \ref{moose-listing} and the sections
below provide various examples of using some of these settings.

!extension-settings moose-listing caption=Settings available when capturing text from a file with the `listing` command.

### Complete Files
You can include complete files from the repository. For example, the
following creates the code listing in \ref{complete}.

```markdown
!listing framework/src/kernels/Diffusion.C id=complete caption=Code listing showing a complete file.
```

!listing framework/src/kernels/Diffusion.C id=complete caption=Code listing showing a complete file.

### Single Line Match
It is possible to show a single line of a file by including a snippet that allows the line to be
located within the file. If multiple matches occur only the first match will be shown. For example,
the call to `addClassDescription` can be shown by adding the following.

```markdown
!listing framework/src/kernels/Diffusion.C id=line caption=Code listing for a single line of a file. line=addClassDescription
```

!listing framework/src/kernels/Diffusion.C id=line caption=Code listing for a single line of a file. line=addClassDescription

### Range Line match
Code starting and ending on lines containing a string is also possible by using the 'start' and
'end' options. If 'start' is omitted then the snippet will start at the beginning of the file.
Similarly, if 'end' is omitted the snippet will include the remainder of the file content.

```markdown
!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=Kernels end=Executioner id=range caption=Code block including lines within a range.
```

!listing test/tests/kernels/simple_diffusion/simple_diffusion.i start=Kernels end=Executioner id=range caption=Code block including lines within a range.

## Input File Block
Like for C++ files, [MOOSE] input files also have additional capability, mainly the "block" setting (see \ref{moose-input-listing} for a complete list). Including the block name the included content will be limited to the content matching the supplied name. Notice that the supplied name may be approximate; however, if it is not unique only the first match will appear.

```markdown
!listing test/tests/kernels/simple_diffusion/simple_diffusion.i block=Kernels id=input caption=Code listing of [MOOSE] input file block.
```

!listing test/tests/kernels/simple_diffusion/simple_diffusion.i block=Kernels id=input caption=Code listing of [MOOSE] input file block.

!extension-settings moose-input-listing caption=List of available settings when including [MOOSE] input (*.i) files.

## Fenced Code Blocks

The `!listing` command can also be utilized to wrap inline fenced code blocks, by placing the command on the line before the fenced code as shown in \ref{fenced}.

!listing id=fenced caption=Example markdown of a fenced code block with the listing command.
~~~markdown
!listing id=combo caption=That's amazing! I've got the same combination on my luggage!
```c++
int combo = 12345;
```
~~~

The available settings for the `!listing` command is reduced from the versions that include files,
because a majority of the options to apply. The complete list is provided in \ref{moose_fenced_code_block}.

!extension-settings moose_fenced_code_block caption=List of available settings when including using the `listing` command with fenced code blocks.

## Extension Configuration
The configuration options for ListingExtension extension are include in \ref{ListingExtension}.

!extension ListingExtension
