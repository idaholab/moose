# NEML2 syntax

The `[NEML2]` block in the MOOSE input file is the entry point for defining NEML2 material model(s). All parameters are listed at [the bottom of the page](syntax/NEML2/index.md#syntax-list).

This page is user-facing, i.e., the [Input file syntax](syntax/NEML2/index.md#input-syntax) section explains how to write the MOOSE input file in order to connect MOOSE to NEML2 for material modeling.

For developers, please refer to the [NEML2Action](NEML2Action.md) and [NEML2ActionCommon](NEML2ActionCommon.md) pages regarding what objects are constructed by underlying Action.

## Input File Syntax id=input-syntax

!listing test/tests/neml2/elasticity.i block=NEML2

## Inspect NEML2 information

The MOOSE solid-mechanics module also provides a command-line option to inspect the NEML2 material model _without_ running the entire simulation. This is achieved using the `--parse-neml2-only` command-line argument, i.e.

```bash
solid_mechanics-opt -i input.i --parse-neml2-only
```

## List of parameters id=syntax-list

### Common parameters

!syntax parameters /NEML2/NEML2ActionCommon

### Parameters specific to each sub-block

!syntax parameters /NEML2/NEML2Action
