# PySyntax Extension

This extension provides the ability to extract python documentation from within a markdown file. The
intended use is for documenting complete python modules and packages. The complete list of available
configuration items for the extension are given in [pysyntax-config]

!devel settings module=MooseDocs.extensions.pysyntax object=PySyntaxExtension id=pysyntax-config
                caption=Configuration items for the pysyntax extension.

## Class Documentation

The "class" command of the extension provides complete documentation for a python class object. For
example, the use of the command to generate class documentation for the `Extension` object for the
pysyntax extension is shown in [pysyntax-class-example].

!devel example id=pysyntax-class-example
               caption=Example use of "pysyntax class" command.
!pysyntax class name=MooseDocs.extensions.pysyntax.PySyntax

The command is designed to operate as a single command within a page. For example, the
[moosetree-Node] contains a single command that generates the complete documentation page:
[moosetree/Node.md].

!listing moosetree/Node.md id=moosetree-Node
         caption=Use of "pysyntax class" command to create complete documentation page for a python class.
