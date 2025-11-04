# Template Extension

The template extension provides the basic capability for creating template MooseDocs markdown
files. For example, this extension is used by the [extensions/sqa.md] to help minimize the burden of
creating SQA documentation. Templates allow custom substitutions to be made into
markdown files, using two methods:

- +Key/value pairs+: Simple substitution of key/value pairs via a syntax `{{keyname}}`.
- +Fields+: Substitution of entire blocks of markdown text/input.

The commands associated with the template extension are the following:

- `template load`: Loads a template file. This is required to use any templates.
- `template field`: Declares a template "field" that must be defined with the `template item` command.
- `template item`: Defines a template "field" that was declared with the `template field` command.

## Configuration

The available configuration options for the extension are given
in [config-template-ext].

!devel settings id=config-template-ext
                caption=Configuration options for the template extension.
                module=MooseDocs.extensions.template
                object=TemplateExtension

The "args" configuration option can be used to set default values for key/value pairs. For example, a
template argument "company" exists in the configuration for MOOSE for use in the following examples:

!listing language=yaml
MooseDocs.extensions.template:
    args:
        company: Ford

## Using a Template id=using-templates

!!! NOTE
This section must be before the create section to have the example render correctly. If you
switch the order then the template field in the example is replaced rather than the one loaded
from the file. Re-ordering the sections was easier than hacking in a fix to the devel extension to
allow it.
!!!

To use a template, it must be loaded with the required fields defined. Assuming that the example
in [create-template] is named "example.md.template", it can be loaded and modified as shown in
[load-template].

!devel! example id=load-template
                caption=Example of loading and modifying template markdown file.
!template load file=example.template.md model=Mustang

!template! item key=details
The {{model}} begin production in 1964.
!template-end!
!devel-end!

Notice that the command includes a key/value argument "model=Mustang". Any keys
not given a value in the configuration must have their values specified as arguments
in this way. Additionally, values specified in the configuration may be overridden
in this way as well. Thus, in this example
it would be possible to override the default for "company" by also supplying it to the load command.

As opposed to "keys", "fields" have their value defined by the `template item`
command. Here the field named `details` (declared in [create-template]) has
its value defined. Note that field definitions may use key/values in their
body.

!alert! warning title=Including text with key/value pairs
Field definitions via `template item` may use key/value pairs in their body; however,
if `!include` is used to include a portion of another file containing the usage
of a key/value pair, then the text `{{keyname}}` will be rendered literally
instead of making the substitution, even if `keyname` exists in both files. For
example, the following does not render properly:

- `file1.md`:

  ```
  !template load template1.md.template keyname=value

  !template! item key=somefield1
  !! some-include-marker-begin
  Some text with {{keyname}}.
  !! some-include-marker-end
  !template-end!
  ```
- `file2.md`:

  ```
  !template load template2.md.template keyname=value

  !template! item key=somefield2
  !include file1.md start=some-include-marker-begin end=some-include-marker-end
  !template-end!
  ```

!alert-end!

!alert! tip title=Multi-paragraph commands
Recall that MooseDown provides the syntax

```
!somecommand!
Paragraph 1

Paragraph 2
!somecommand-end!
```

for a command `somecommand` that needs to include blank lines (for example, when
there are multiple paragraphs or nested commands).
!alert-end!

The complete list of settings for the "template item" command are provided in
[template-item].

!devel settings id=template-item
                caption=Available settings for the 'template item' command.
                module=MooseDocs.extensions.template
                object=TemplateItemCommand

## Creating a Template

To create a template file, start by creating a new file with the extension `.md.template`.

!alert note title=Template extension convention
Usage of the file extension `.md.template` avoids the raw template file from being
rendered and added to documentation.

Next, add content as in any other `.md` file, using the syntax `{{keyname}}` where
you want to substitute a template parameter with the key name `keyname`. Then when
the template is loaded, the key `keyname` will be required to have its value defined.

Use `template field` to declare a "field" with the name specified by the `key`
argument (do not confuse with key/value arguments). The text following this command
is the text that appears in an error box when the template is loaded without
defining the field's value via `template item`.

The complete list of settings
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

!devel settings id=template-field
                caption=Available settings for the 'template field' command.
                module=MooseDocs.extensions.template
                object=TemplateFieldCommand
