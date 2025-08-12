# AbaqusInputObjects

## Overview

- Purpose: Convert the parser’s syntax tree into a validated model of parts, instances, assemblies, sets, user elements, and steps that downstream code can consume.
- Scope: Semantic interpretation of `*Node`, `*Element`, `*User Element`, `*Nset`, `*Elset`, `*UEL PROPERTY`, and `*Step` (`*Boundary`, `*Static`, etc.).

## Model Types

- `Part`: Owns part-local nodes, elements, UEL definitions, properties, and sets; maps part-local Abaqus IDs to contiguous indices.
- `Instance`: Created under an `Assembly`; references a `Part` and records offsets mapping part-local indices to global model indices (plus rigid transforms if specified).
- `Assembly`: Holds one or more `Instance`s. Multiple instances are supported; scoping rules below disambiguate IDs across instances.
- `Step`: Interprets step options (time data, boundaries) and maintains `_bc_var_node_value_map` used by BC application.
- `Model` (Flat/Assembly): Coordinates parsing across Parts, Steps, and, for assemblies, Instances.

## Index Lookups

- APIs: `Model::getNodeIndex(key, instance)` and `Model::getElementIndex(key, instance)`.
  - FlatModel: errors if `instance != nullptr`; accepts clean integer keys only.
  - AssemblyModel:
    - If `instance` is provided: `key` must be a clean integer; lookup uses the instance’s part-local maps and adds the instance’s global offset.
    - If `instance == nullptr`: supports `instanceName.id` (inline) or plain integers.

## Instance Access

- `Model::getInstance(name)`: Returns a reference to the named `Instance` or errors if not found. FlatModel errors.

## Sets and Scoping (Multi-Instance)

- Part-level sets (`*Nset/*Elset` under `*Part`):
  - Interpreted against the owning `Part` (clean integers, nested set names, `GENERATE`).
  - During instantiation, part-level sets are merged into model-level sets for each instance with proper index offsets applied.

- Assembly-level sets (`*Nset/*Elset` under `*Assembly`):
  - Nodesets must be instance-scoped when using numeric IDs:
    - Header `instance=NAME` with clean integer tokens, or
    - Inline instance-qualified tokens `NAME.id` without a header `instance=`.
  - Element sets follow the same scoping rules. Additionally:
    - Header `instance=NAME` is required for numeric-only tokens at assembly scope; alternatively use inline `NAME.id`.
    - Referencing an existing set by name is supported (copies entries into the target set).
  - `GENERATE` is supported for both nodal and element sets; at assembly scope it requires an instance context (header or inline for nodal; header for element unless using inline tokens).
  - Ambiguity: header `instance=` and inline `NAME.id` are mutually exclusive within the same set; using both causes an error.

- Elset-to-Nset copy:
  - `*Nset, elset=S, instance=NAME` copies nodes from elements in `S` taken from the specified instance; indices are shifted by the instance’s node offset.

## Boundary Conditions

- `*Boundary` mirrors index lookup rules:
  - With header `instance=NAME`, single-node entries must be clean integers; resolved within that instance.
  - Without a header instance, inline `NAME.id` may be used.
  - Values are stored in `Step::_bc_var_node_value_map` keyed by Abaqus variable id.

## Examples (Assembly Scope)

- Mixed nodeset across two instances using inline tokens:
  - `*Nset, nset=NS_MIXED`
  - `I1.1, I2.2`

- Assembly-level element set for a specific instance (inline token):
  - `*Elset, elset=ES_I2`
  - `I2.1`

- Nodeset from an elset for a specific instance:
  - `*Nset, nset=NS_FROM_ES, elset=EALL, instance=I2`

## Design Notes

- Separation of concerns: `AbaqusInputParser` builds a neutral AST; this layer interprets and validates semantics.
- Multi-instance: Implemented by resolving IDs against instance-local maps with offsets, plus explicit scoping at assembly scope.
- Error reporting: Errors use `mooseError` with precise messages; consider adding source location to nodes to improve diagnostics.
