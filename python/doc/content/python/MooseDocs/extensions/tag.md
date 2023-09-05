# Tagging Extension

The tagging extension provides the "tag" command, which provides a mechanism for setting a set of
tags made up of arbitrary key:value pairs for categories/labels associated with a given markdown page.
The provided metadata is then injected into a compatible user-provided JavaScript file, which then
allows for the filtering of tagged pages against the defined categories in the generated website.
The configuration of the extension contains an optional allowed key list, which is checked against
during the invocation of the command. The available configuration items for this extension are provided
in [tagging-config].

!alert! warning title=The tagging extension is experimental!
This extension is considered to be in an experimental state, as several manual steps are required in
order to obtain both a set of tagged pages, and a rendered page for filtering against those tags. If
documentation tagging features are desired, please request assistance from the MOOSE Framework
development team.
!alert-end!

!devel settings module=MooseDocs.extensions.tagging
                object=TaggingExtension
                id=tagging-config
                caption=Configuration items for the tagging extension.

An example of the usage of extension settings from [tagging-config] in a configuration file is shown
below.

```
Extensions:
    ...
    MooseDocs.extensions.tagging:
        active: True
        js_file: tagging.js
        allowed_keys:
            - application
            - foo
            - simulation_type
            - fiscal_year
```

## Basic Tag

There is only one version of the tag command, demonstrated below. The available settings for the
tag command are listed in [tagging-settings].

```
!tag name=tagging_one pairs=application:test foo:bar simulation_type:triage fiscal_year:2023
```

If either the 'name' or the 'pairs' parameters are left out of the invocation of the tag command, an
error message is generated that directs the user to the affected markdown file.

!devel settings module=MooseDocs.extensions.tagging
                object=TaggingCommand
                id=tagging-settings
                caption=Available settings for the `!tag` command.
