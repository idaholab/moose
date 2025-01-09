# Listing Extension

The listing extension provides a mechanism for creating source code listings. The system allows for
the inclusion of local content as well as complete or partial snippets of the desired source code and
includes the ability to parse MOOSE input files and separate out blocks. The main purpose is to avoid
copying code or input syntax into the documentation to avoid out-of-date content.

The extension provides the `!listing` command (see [command.md]), which allows for numbered captions
to be applied, the [extensions/floats.md] provides additional details. The following table lists the
available configure options for the extension.

!devel settings module=MooseDocs.extensions.listing object=ListingExtension

The `!listing` command has the capability to include text from local content and arbitrary files
(such as source code files). There is a wide range of settings that are available to specialize how
the code is imported.  The complete list of available settings are provided in [moose-listing] and
the sections below provide various examples of using some of these settings.

## Local Listing Content

It is possible to create a listing using local content. This is done by using the `!listing` command
without any arguments with the desired content following the command, see [command.md] for details
to how content is defined. The available settings for this command are given in
[listing-local-settings].

!devel! example id=example-listing-local caption=Example listing with content from local markdown.
!listing id=local caption=A function for adding 42. language=cpp
double add_forty_two(const double y) {
  y += 42;
  return y;
}
!devel-end!

!devel settings module=MooseDocs.extensions.listing
                object=LocalListingCommand
                id=listing-local-settings
                caption=Settings available when creating a listing from local content.

## File Content

You can include complete files from the repository. For example, the following creates the code
listing in [example-listing-complete]. The available settings for this command are given in
[moose-listing].

!devel! example id=example-listing-complete caption=Example for showing a complete file.
!listing framework/src/kernels/Diffusion.C
!devel-end!

!devel settings module=MooseDocs.extensions.listing
                object=FileListingCommand
                id=moose-listing
                caption=Settings available when capturing text from a file with the `listing` command.

### Regular Expression Match

Regular expressions can be utilized to extract specific content from files. [example-listing-re]
uses a regular expression to extract the content of a class method.

!devel! example id=example-listing-re caption=Example listing using the "re" setting.
!listing framework/src/kernels/Diffusion.C
         re=Real\sDiffusion::computeQpResidual.*?^}
!devel-end!

### Single Line Match

It is possible to show a single line of a file by including a snippet that allows the line to be
located within the file. If multiple matches occur only the first match will be shown. For example,
the call to `addClassDescription` can be shown by adding the following.

!devel! example id=example-listing-line caption=Example for displaying a single line from a file.
!listing framework/src/kernels/Diffusion.C line=computeQp
!devel-end!

### Range Line Match

Code starting and ending on lines containing a string is also possible by using the 'start' and
'end' options. If 'start' is omitted then the snippet will start at the beginning of the file.
Similarly, if 'end' is omitted the snippet will include the remainder of the file content.

!devel! example id=example-listing-start-end caption=Example listing using the "start" and "end" settings.
!listing moose/test/tests/kernels/simple_diffusion/simple_diffusion.i
         start=Kernels
         end=Executioner
!devel-end!

## Input File Content id=moose-input

Like C++ files, [MOOSE] input files also have additional capability, mainly the "block" and "remove"
settings (see [hit-listing-settings] for a complete list). The included content will be limited to
the input block matching the supplied name. Notice that the supplied name may be approximate;
however, if it is not unique only the first match will appear. Similarly, input file blocks may be
removed from the displayed content. In addition to block names, the "remove" setting also accepts
the names of individual input parameters. However, the *full* path to the parameter must be
specified. [example-listing-input] demonstrates proper use of the `block` and `remove` syntax.

!devel! example id=example-listing-input caption=Example use of "block" and "remove" settings for input files.
!listing moose/test/tests/kernels/simple_diffusion/simple_diffusion.i
         block=Kernels
         id=input-block
         caption=Code listing of [MOOSE] input file block.

!listing moose/test/tests/kernels/simple_diffusion/simple_diffusion.i
         remove=left Executioner/petsc_options_iname Executioner/petsc_options_value
         id=input-remove
         caption=Code listing of a [MOOSE] input file with blocks and parameters removed.
!devel-end!

!alert note title=The "block" and "remove" settings apply to all HIT formatted files.
The settings given in [hit-listing-settings] are available to all [!ac](HIT) formatted listings, including
[test specification files](TestHarness.md).

!devel settings module=MooseDocs.extensions.listing
                object=InputListingCommand
                id=hit-listing-settings
                caption=Settings available for input file listings.

## Diffing Content

You can produce a diffed listing between two files using the "diff" setting.

!devel! example id=example-listing-diff caption=Example listing using the "diff" setting.
!listing framework/src/kernels/Diffusion.C
         diff=framework/src/kernels/ADDiffusion.C
!devel-end!

The prefixes for the links after the listing (when using the "link" option, which defaults to true)
can be modified using the "before_link_prefix" and "after_link_prefix" settings.

!devel! example id=example-listing-diff-prefixed caption=Example listing using the "diff" setting with overridden link prefixes.
!listing framework/src/kernels/Diffusion.C
         diff=framework/src/kernels/ADDiffusion.C
         before_link_prefix=Diffusion without AD:
         after_link_prefix=Diffusion with AD:
!devel-end!
