# NEML2Action

!syntax description /NEML2/NEML2Action

!alert note
This page is developer-facing. Users please refer to the [NEML2 syntax](syntax/NEML2/index.md) documentation.

## Overview

This is the primary action responsible for constructing objects for transferring data back and forth between MOOSE and NEML2, as well as for executing the NEML2 model.

Four types of objects are constructed by this action:

- +Gatherer+: Object responsible for gathering MOOSE data.
- +Retriever+: Object responsible for retrieving NEML2 data and assigning it back to MOOSE data structures.
- +Index generator+: Object responsible for generating the element-to-batch-index map.
- +Executor+: Object responsible for sending data gathered by gatherers to NEML2, execute the NEML2 model, and have the outputs ready for retrieval by retrievers.

Currently supported objects are summarized below.

| Type            | MOOSE object(s)                                                 |
| :-------------- | :-------------------------------------------------------------- |
| Gatherer        | [MOOSEMaterialPropertyToNEML2](MOOSEMaterialPropertyToNEML2.md) |
|                 | [MOOSEVariableToNEML2](MOOSEVariableToNEML2.md)                 |
|                 | [MOOSPostprocessorToNEML2](MOOSEPostprocessorToNEML2.md)        |
| Retriever       | [NEML2ToMOOSEMaterialProperty](NEML2ToMOOSEMaterialProperty.md) |
| Index generator | [NEML2BatchIndexGenerator](NEML2BatchIndexGenerator.md)         |
| Executor        | [NEML2ModelExecutor](NEML2ModelExecutor.md)                     |

Multiple gatherers and retrievers may be created by this action, but each instance of the action creates one and only one index generator and executor.  Each step performed by this action is explained in one of the following sections, in the order of execution.

## Applying common parameters

Recall that input file parameters in the common area under the `[NEML2]` block defined by the [NEML2ActionCommon](NEML2ActionCommon.md) are shared by all sub-blocks. The first step is to retrieve the common parameters and apply them to this sub-block. This happens in the constructor of the action. Note that parameters defined in the sub-block overrides the parameters defined in the common area.

## Creating the NEML2 model

By the time this action "acts", the NEML2 input file must have already been parsed and loaded by [NEML2ActionCommon](NEML2ActionCommon.md). Therefore, the NEML2 model associated with this action can be directly created using the name given by [!param](/NEML2/model).

## NEML2 model introspection

If [!param](/NEML2/verbose) is set to `true`, a high-level summary of the NEML2 model is print to the console, including model name, evaluation device, as well as a table summarizing the model's variables, parameters, and buffers.

NEML2 model allows introspection, i.e., variable and parameter names, tensor types, and storage sizes can be inferred after the model is loaded. Five maps are set up upon introspection of the NEML2 model:

1. A map between MOOSE data structures and NEML2 +input variables+.
2. A map between MOOSE data structures and NEML2 +model parameters+.
3. A map between MOOSE data structures and NEML2 +output variables+.
4. A map between MOOSE data structures and NEML2 output +derivatives+.
5. A map between MOOSE data structures and NEML2 +parameter derivatives+.

These maps are used later to create objects for transferring data between MOOSE and NEML2.

## Creating MOOSEToNEML2 gatherers id=gatherer

This step involves the creation of `MOOSEToNEML2` gatherers for NEML2 input variables and model parameters. The first two maps mentioned in the previous section are used. The maps are looped through to create objects one-by-one.

Note that the correct type of object is selected/created relying on NEML2 model introspection. For example, if a NEML2 input variable is of type `SR2`, we can infer that the corresponding MOOSE data structure should be `SymmetricRankTwoTensor`. According to the user-specified [!param](/NEML2/moose_input_types), we can deduce where the MOOSE data comes from. (Though in this case `MATERIAL` is the only viable option, as a `SymmetricRankTwoTensor` cannot come from a MOOSE variable nor a MOOSE postprocessor).

## Creating the NEML2 index generator and model executor

Next, a [NEML2BatchIndexGenerator](NEML2BatchIndexGenerator.md) and a [NEML2ModelExecutor](NEML2ModelExecutor.md) excutor are created. The list of [!param](/UserObjects/NEML2ModelExecutor/gatherers) and [!param](/UserObjects/NEML2ModelExecutor/param_gatherers) are automatically filled out by this action.

## Creating NEML2ToMOOSE retrievers

The last step is creating the `NEML2ToMOOSE` retrievers to retrieve NEML2 model outputs (and their derivatives) back to MOOSE. This step is similar to [creating MOOSEToNEML2 gatherers](NEML2Action.md#gatherer).

!syntax parameters /NEML2/NEML2ActionCommon

These input parameters correspond to the sub-blocks under the `[NEML2]` block. The usage of the `[NEML2]` block is explained in details in the [NEML2 syntax](syntax/NEML2/index.md) documentation.
