# Template Extension

The template extension provides the basic capability for creating template MooseDocs markdown
files. This extension is used by the [extensions/sqa.md] to help minimize the burden of
creating SQA documentation. The available configuration options for the extension are given
in [config-template-ext].

!devel settings id=config-template-ext
                caption=Configuration options for the template extension.
                module=MooseDocs.extensions.template
                object=TemplateExtension

The "args" configuration option can be used to set default template arguments. For example, a
template argument "company" exists the configuration for MOOSE for use in the following examples.

!listing language=yaml
MooseDocs.extensions.template:
    args:
        company: Ford

## Using a Template

!!! NOTE
This section must be before the create section to have the example render correctly. If you
switch the order then the template field in the example is replaced rather than the one loaded
from the file. Re-ordering the sections was easier than hacking in a fix to the devel extension to
allow it.
!!!

To use a template it must be loaded and then the required fields defined. Assuming that the example
in [create-template] was named "example.md.template", it can be loaded and modified as shown in
[load-template].

!devel! example id=load-template
                caption=Example of loading and modifying template markdown file.
!template load file=example.template.md model=Mustang

!template! item key=details
The {{model}} begin production in 1964.
!template-end!
!devel-end!

Notice that the command includes a key/value input "model=Mustang". All key/value supplied that
are not in settings for the command are passed in as template arguments. These arguments are
used to update the default arguments supplied by the extension configuration. Thus, in this example
it would be possible to override the default for "company" by also supplying it to the load command.

The complete list of settings for the "template item" command are provided in
[template-item].

!devel settings id=template-item
                caption=Available settings for the 'template item' command.
                module=MooseDocs.extensions.template
                object=TemplateItemCommand

## Creating a Template

A template is a MooseDocs file with special sections that are designed to be replaced by another file
that loads the template. Begin by creating a file and associated content that is constant, such as
the headings or introduction. Then within this content begin by adding required or optional fields
that are intended to be customized in the file that loads the template. The complete list of settings
for the "template field" command are provided in [template-field].

!devel! example id=create-template
                caption=Example template markdown file.
# {{model}} Information

## Introduction

The {{model}} is a car manufactured by {{company}}.

## Details

!template field key=details
Provide detailed information regarding the specific {{company}} model.

!devel-end!

!alert tip title=Template File Extension
Within MOOSE documentation the '.md.template' file extension is used for template files. This will
avoid the template itself from getting rendered and copied to output destination.

!devel settings id=template-field
                caption=Available settings for the 'template field' command.
                module=MooseDocs.extensions.template
                object=TemplateFieldCommand
