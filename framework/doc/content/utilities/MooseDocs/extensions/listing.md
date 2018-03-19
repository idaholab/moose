# Listing Extension

The listing extension provides a mechanism for creating source code listings. The system allows for
the inclusion of local content as well as complete or partial snippets of the desired source code and
includes the ability to parse MOOSE input files and separate out blocks. The main purpose is to avoid
copying code or input syntax into the documentation to avoid out-of-date content.

The extension provides the `!listing` command (see [command.md]), which allows for numbered captions
to be applied, the [extensions/floats.md] provides additional details. The following table list the
available configure options for the extension.

!devel settings module=MooseDocs.extensions.listing object=ListingExtension

The `!listing` command has the capability to include text from local content and arbitrary files
(such as source code).  files. There is a wide range of settings that are available to specialize how
the code is imported.  The complete list of available of settings are provided in [moose-listing] and
the sections below provide various examples of using some of these settings.

## Local Listing Content

It is possible to create a listing using local content. This is done by using the `!listing` command
without any agruments with the desired content following the command, see [command.md] for details
to how content is defined. The available settings for this command are given in
[listing-local-settings].

!devel! example id=example-listing-local caption=Example listing with content from local markdown.
!listing id=local caption=A function for adding 42. language=cpp
double add_forty_two(const double y);
y += 42;
return y;
!devel-end!

!devel settings module=MooseDocs.extensions.listing
                object=LocalListingCommand
                id=listing-local-settings
                caption=Settings available when creating a listing from local content.

## File Content

You can include complete files from the repository. For example, the following creates the code
listing in [example-listing-complete].

!devel settings module=MooseDocs.extensions.listing
                object=FileListingCommand
                id=moose-listing
                caption=Settings available when capturing text from a file with the `listing` command.


!devel! example id=example-listing-complete caption=Example for showing a complete file.
!listing framework/src/kernels/Diffusion.C
!devel-end!

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

Like for C++ files, [MOOSE] input files also have additional capability, mainly the "block" setting
(see [example-listing-input] for a complete list). Including the block name the included content
will be limited to the content matching the supplied name. Notice that the supplied name may be
approximate; however, if it is not unique only the first match will appear.

!devel! example id=example-listing-input caption=Example use of "block" setting for input files.
!listing moose/test/tests/kernels/simple_diffusion/simple_diffusion.i
         block=Kernels
         id=input
         caption=Code listing of [MOOSE] input file block.
!devel-end!

!devel settings module=MooseDocs.extensions.listing object=InputListingCommand

[MOOSE]: www.mooseframework.org
