# ConstructiveSolidGeometry

The `[ConstructiveSolidGeometry]` block generates a [!ac](CSG) model from the mesh generators in the input file and configures the [!ac](JSON) output.
It is the input file equivalent of the `--csg-only` command line option, described in [syntax/CSG/index.md#how-to-generate-a-csg-model].
For a description of [!ac](CSG) in MOOSE and of the contents of the output file, see [syntax/CSG/index.md].

## Example Input Syntax

```
[ConstructiveSolidGeometry]
  output_file = 'my_model_csg.json'
  expand_units = false
[]
```

The presence of the block enables [!ac](CSG)-only mode, so that the mesh generators call `generateCSG()` instead of `generate()` and no finite element mesh is produced.
Setting `generate_csg = false` keeps the block in the input file while running the input normally.

The [!ac](JSON) file is named by `output_file`. By default, it is the name of the input file with `_csg` appended.

All [engineering units](syntax/CSG/index.md#engineering-units) are expanded into standard [!ac](CSG) components before output by default.
Setting `expand_units = false` retains them as engineering units in the output.

## Command Line Precedence

Command line options take precedence over the block:

- An output file name passed to `--csg-only` overrides `output_file`.
- `--mesh-only`, `--split-mesh`, and `--parse-neml2-only` disable [!ac](CSG)-only mode with a warning.
- When `--csg-only` is used with `generate_csg = false`, [!ac](CSG)-only mode is enabled with a message noting that the command line takes precedence. The `output_file` and `expand_units` parameters still apply.

## Input Parameters

!syntax parameters /ConstructiveSolidGeometry/CSGAction heading=None
