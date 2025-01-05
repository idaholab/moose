# IfElse Extension

This extension provides basic functionality for markdown based conditional statements to allow for
the rendered content to have conditional requirements. The extension adds three commands: `if`,
`elif`, and `else`. These commands must be used in the traditional order, where the first is `if`,
optionally followed by any number of `elif` commands and optionally a final `else` command.

A complete list of the available configuration options are provided in [ifelse-config] and the
following sections demonstrate the use of the commands.

!devel settings module=MooseDocs.extensions.ifelse object=IfElseExtension id=ifelse-config
                caption=Available configuration options for the ifelse extension.

The "modules" configuration item provides a list of python modules to search when calling
conditional functions. The default behavior of the module is equivalent to adding the following
to the "modules" item.

!listing language=yaml
MooseDocs.extensions.ifelse:
    modules:
        - MooseDocs.extensions.ifelse

As such, any function within this module can be called by the "function" setting within the
statements. This setting and the associated function is discussed further in the following section.

## Simple if Statement

The "if" command requires that a function to evaluate be supplied using the "function" setting.  This
function must exist within the modules loaded by the extension using the "modules" configuration item
(see [ifelse-config]).

This extension was originally created to allow content to depend on the application name, as
such a scenario will be used to demonstrate use of the syntax. The most basic use is a single
"if" statement. When the supplied function returns `True` the content is
displayed (see [if-true], when `False` the content is ignored (see [if-false]).

!devel example id=if-true caption=Single "if" command, when the function returns `True` the
                                  content is displayed.
!if function=hasMooseApp('MooseApp')
The documentation contains 'MooseApp' information.

!devel example id=if-false caption=Single "if" command, when the function returns `False` the
                                   content is ignored.
!if function=hasMooseApp('UnknownApp')
The documentation contains 'UnknownApp' information.

It is possible to flip the state of the return value and create an "if not" statement by prefixing
the function with an exclamation mark, as shown in [if-not].

!devel example id=if-not
               caption=Single "if not" statement, when the function returns `False` the content is displayed.
!if function=!hasMooseApp('UnknownApp')
The documentation does not contain 'UnknownApp' information.

The input to the "function" setting is expected to a function that is available within the
modules loaded by the extension. The function must accept the `IfElseExtension` object as the
first argument. The arguments supplied in the markdown text are appended.

For example, the `hasMooseApp` function shown in the above examples is defined as follows in the
extension.

!listing MooseDocs/extensions/ifelse.py start=def hasMooseApp end=return ext include-end=True

The complete list of settings available for the "if" command are provided in [if-settings], the
"function" setting is required.

!devel settings module=MooseDocs.extensions.ifelse object=IfCommand id=if-settings
                caption=Available settings for the "if" command.


## Compound if/elif/else Statements

Creating `if`-`elif`-`else` statements is accomplished by using an `if` command followed by any
number of `elif` commands and then a final (optional) `else` command. It is important to understand
that the implementation actually uses separate [commands](command.md). As such, each command (the
`if`, `elif`, and/or the `else`) can use the inline or block version of the command definition. The
extension will enforce that commands occur in the expected order.

!devel! example id=if-elif-else caption=A `if elif else` statement that mixes the command
                                        between inline and block syntax.
!if function=hasMooseApp('UnknownApp')
This 'UnknownApp' was found, how did you do that?

!elif! function=hasMooseApp('MooseApp')
You application includes objects registered by MOOSE as follows.

```
registerMooseObject('MooseApp', Diffusion);
```
!elif-end!

!else
Your install is messed up!
!devel-end!
