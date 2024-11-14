# ChainControlParsedFunctionWrapper

This class wraps `libMesh::ParsedFunction` for use of various parsed
[ChainControls](syntax/ChainControls/index.md).

For valid syntax for the function expression, see the
[function parser site](http://warp.povusers.org/FunctionParser/).

The user-defined symbols may include the following:

- `bool` or `Real` [/ChainControlData.md]
- [Functions](syntax/Functions/index.md)
- Scalar variables
- Constant values

Pre-defined symbols that may be used in the function expression are `x`, `y`, `z`, and `t`,
representing the corresponding spatial coordinates and time value.
