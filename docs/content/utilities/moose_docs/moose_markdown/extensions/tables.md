# Tables

The TableExtension provides the ability to place captions about traditional markdown tables via
the `!tables` command. Additionally, this command enables tables to be numbered as detailed
in the [Numbered Floats](extensions/numbered_floats.md) page.

To create a captioned table, as in \ref{testing}, utilize the `!table` command on the
line above the markdown table content, as shown in below.

```markdown
!table id=testing caption=This is an example table with a caption.
| 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|----|
| 2 | 4 | 6 | 8 | 10 |
```

!table id=testing caption=This is an example table with a caption.
| 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|----|
| 2 | 4 | 6 | 8 | 10 |

The available settings for the `!tables` command are included in \ref{moose_table}.

!extension-settings moose_table caption=Command settings for `!table` command.
