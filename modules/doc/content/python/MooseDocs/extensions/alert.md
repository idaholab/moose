# Alert Extension

The alert extension provides the "alert" command, which provides a mechanism
for making alert boxes to highlight important items within the markdown text.
The available configuration items for this extension are provided in [alert-config].

!devel settings module=MooseDocs.extensions.alert
                object=AlertExtension
                id=alert-config
                caption=Configuration items for the alert extension.

## Basic Alert

There are four versions of alerts---error, warning, note, and tip---all of which are demonstrated
in [alert-example-all]. The available settings for the alert command are listed in
[alert-settings].

!devel settings module=MooseDocs.extensions.alert
                object=AlertCommand
                id=alert-settings
                caption=Available settings for the `!alert` command.

!devel! example id=alert-example-all
               caption=Example of the three types of alerts: error, warning, and note.
!alert error
This is an error alert.

!alert warning
This is a warning alert.

!alert note
This is a note alert.

!alert tip
This is a tip alert.
!devel-end!

## Alert Title

[alert-example-title] demonstrates the use of title setting within the `!alert` command,
which can contain inline markdown (see [/core.md#core-inline]).

!devel! example id=alert-example-title
                caption=Example use of the title setting wihtin the `!alert` command.
!alert warning title=This +is+ an =error=.
Do not do this, *it is bad*, umk.
!devel-end!

## Block Alert

The previous examples show the inline version (see [/core.md#core-inline]) of the
alert command. Since, this command was built using the [command.md] there also exists a
block version, which is demonstrated in [alert-example-block].

!devel! example id=alert-example-block
                caption=Example showing the block version of the `!alert` command.
!alert! note
> This is a block version
> that allows the use of
> block markdown syntax
> to be used.
!alert-end!
!devel-end!

## Alert without Title

It is also possible to create an alert without a title, which may be useful for simply highlighting
regions of text, this is done by ommitting the title as well as setting the 'prefix' setting
to false, as shown in [alert-example-no-title].

!devel! example id=alert-example-no-title
                caption=Example use of the title setting wihtin the `!alert` command.
!alert note prefix=False
This is an alert without a title,
which might be useful for highlighting text.
!devel-end!
