# DateTime Extension

The DateTime extension provides the "datetime" command that provides a mechanism
for date and time operations. This extension is based on the
`datetime` package that is part of the standard python library.

!devel settings module=MooseDocs.extensions.datetime
                object=DateTimeExtension
                id=datetime-config
                caption=Configuration items for the datetime extension.

## Today

The "today" sub-command inserts the date when the execution of the documentation build occurs, by
calling the python `datetime.date.today()` function, as demonstrated in [datetime-today-example].
The available settings for the this command are provided in [datetime-today-settings].

!devel settings module=MooseDocs.extensions.datetime
                object=TodayCommand
                id=datetime-today-settings
                caption=Available settings for the `!datetime today` command.

!devel! example id=datetime-today-example
               caption=Example of showing date of build execution.
!datetime today

!datetime today format=%B %d, %Y

Today is [!datetime!today] or in a nicer format: [!datetime!today format=%B %d, %Y].
!devel-end!
