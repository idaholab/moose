# NEML2ActionCommon

!syntax description /NEML2/NEML2ActionCommon

!alert note
This page is developer-facing. Users please refer to the [NEML2 syntax](syntax/NEML2/index.md) documentation.

## Overview

This action corresponds to the common area directly underneath the `[NEML2]` block in the input file. It defines common parameters that are also applied to each of the sub-block.

The only responsibility of this action is to parse the given NEML2 input file specified by the [!param](/NEML2/input) and load all input options into the NEML2 factory, so that other objects can create NEML2 models as needed.

This action does not construct any object by itself. All NEML2 object construction is handled by the sub-blocks which correspond to [NEML2Action](NEML2Action.md).

!syntax parameters /NEML2/NEML2ActionCommon

These input parameters correspond to the common area under the `[NEML2]` block. The usage of the `[NEML2]` block is explained in details in the [NEML2 syntax](syntax/NEML2/index.md) documentation.

