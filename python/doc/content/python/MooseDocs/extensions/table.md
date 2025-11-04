# Table Extension

The table extension provides a means for defining tables using traditional markdown syntax, as
shown in [example-basic]. It also adds the ability to create numbered and captioned tables
as in [example-caption].

!devel! example caption=Example of stand-alone table using markdown syntax. id=example-basic
| Heading 1 | Heading 2 |
| - | - |
| Item 1 | Item 2 |
!devel-end!

!devel! example caption=Example of table using markdown syntax displayed as a float. id=example-caption
!table id=table-floating caption=This is a "floating" table.
| Heading 1 | Heading 2 |
| - | - |
| Item 1 | Item 2 |
!devel-end!

The table extension also supports 'left', 'center', and 'right' aligned columns, again using
traditional markdown format.

!devel! example caption=Example showing alignment of columns.
| Left | Center | Right |
| :- | - | -: |
| This text is aligned to the left | This text is centered | This is aligned to the right |
!devel-end!
