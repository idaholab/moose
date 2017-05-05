# MOOSE Developer Extension (DevelExtension)

This extension includes the ability to list MooseDocs markdown configuration options and
commands settings. It also provides markdown syntax for displaying the build status and
current MOOSE package download links. The list of extension level configuration options are
provided in \ref{DevelExtension} below.

!extension DevelExtension title=DevelExtension Configuration Options

## Build Status
!buildstatus https://moosebuild.org/mooseframework/ float=right padding-left=10px

You can add a Civet build status widget to any page using `!buildstatus` command as shown below. The available settings are provided in \ref{moose_build_status}. Currently, this will only work with Civet continuous integration services.

```markdown
!buildstatus https://moosebuild.org/mooseframework/ float=right padding-left=10px
```

!extension-settings moose_build_status caption=Command settings for `!buildstatus` command.


## Package Information
This extension includes a `!moosepackage` command for creating links to the [MOOSE] redistributable
package links. The available settings for this command are listed below in \ref{moose_package_parser}.

!extension-settings moose_package_parser caption=Command settings for `!moosepackage` command.

## Extension Configuration and Settings

The devel extension also includes two commands (`!extension` and `!extension-settings`) for extracting and displaying the extension
configuration options as well as the settings for extension commands. For example, the following
commands are used within this markdown page.

* `!extension DevelExtension` <br>
This command displays the configuration options for the extension as a whole; these are the options
that may be specified in the configuration [YAML] file (e.g., website.yml). The settings for this
command are provided in \ref{moose_extension_config}.

* `!extension-settings moose_package_parser` <br>
This displays the optional key, value pairs that are passed to the individual commands contained
within the extension object. Automatic extraction of settings is only available for python objects
that inherit from MooseCommonExtension. The available settings for this command are shown in
\ref{moose_extension_component_settings}.

For both commands, if no configuration or settings are located the syntax is ignored and nothing
is displayed. This allows for the commands to be present in the markdown regardless if options are
available. But, if the underlying python code is changed to include options, they will display without modification
of the markdown.

!extension-settings moose_extension_config caption=Command settings for `!extension` command.

!extension-settings moose_extension_component_settings caption=Command settings for `!extension-settings` command.
