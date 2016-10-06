# MOOSE Documentation System

As MOOSE and MOOSE-based applications continue to advance having quality documentation rapidly becomes
key to the gaining new users and provide current users with an experience that allows rapid development.
However, creating quality or any documentation at all can be a daunting task that often not achievable,
MOOSE itself has gone through this problem itself, where documentation was limited, scattered among
multiple formats, and difficult to navigate.

To ameliorate this problem a single source documentation system was developed based around fully customizable markdown syntax---[Moose Style Markdown](moose_flavored_markdown.md)---that includes features tailored to documenting MOOSE code:

* Object descriptions and input parameters tables may be automatically generated from applications.
* Lists of cross-referencing links to examples and source code that use or inherit from your objects may be created.
* Tools for placing source code without copying code allow documentation to change with the code, without modification.

The single source markdown files may be converted into various media forms include web-sites (like this one), slide shows, and pdfs (via latex). Most importantly, custom markdown syntax is easily created so it is possible to develop syntax to meet the needs of any project.

The following links provide additional details on the MOOSE documentation system:

* [Setup and Configuration](documentation/setup.md)
* [Creating a Web-Site](documentation/website.md)
* [Building a Presentation](documentation/presentation.md)
* [Generating a PDF](documentation/pdf.md)
* [Moose Style Markdown](documentation/moose_flavored_markdown.md)
