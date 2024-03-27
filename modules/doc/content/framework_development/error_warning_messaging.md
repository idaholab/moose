# Error, Warning, and Informational Messaging

MOOSE provides several macros/functions for emitting errors, warnings, and informational
messages:

- `mooseError(args)`: Emits an error with a message formed by concatenating `args`.
- `mooseDocumentedError(repo_name, issue_num, args)`: Same as `mooseError(args)`, but
  for errors tied to defects documented in an issue tracker. See [#documented_errors].
- `mooseAssert(condition, msg)`: In a debug executable, emits an error with message
  `msg` if the boolean value/expression `condition` is `false`. If not in a debug
  executable, does nothing.
- `mooseWarning(args)`: Emits a warning with a message formed by concatenating `args`.
  If the command-line flag `--error` is supplied, then these warnings are promoted
  to errors.
- `mooseDeprecated(args)`: Emits a warning related to a deprecated object/feature.
  If the command-line flag `--error-deprecated` is supplied, then these warnings
  are promoted to errors.
- `mooseInfo(args)`: Emits an informational message formed by concatenating `args`.
  Example:
- `mooseInfoRepeated(args)`: The same as `mooseInfo(args)` but only prints a message
  once, even if called multiple times.

!alert note title=Warnings in repeating sections
For warnings in sections of the code that are often executed, it is recommended to leverage
the [SolutionInvalidInterface.md], which will only output the warning once but keep track of
the occurence of the problem. It can also force the solver to reject a converged solution that
still presents the warning/invalid condition.

## Documented Errors id=documented_errors

The `mooseDocumentedError(repo_name, issue_num, args)` function is used for errors that are tied to
defects documented in the issue tracker of the framework or an application. This
function is preferred over `mooseError` when it applies. For example, when
there is some missing functionality or incompatibility, one can create an
issue in the issue tracker for that repository, and then a developer can call
`mooseDocumentedError` with the registered repository name `repo_name` (see below), the issue number `issue_num`
(provided as an integer, not a string), and the error message arguments `args`.

To register a repository name for an application, one must use `registerRepository(repo_name, repo_url)`
in their application's `registerAll` method, where `repo_url` corresponds to the
URL of the Github or Gitlab repository; for example,

```
void
ExampleApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  // some statements
  registerRepository("exampleapp", "github.com/someuser/exampleapp");
}
```

One can then call `mooseDocumentedError` with the registered repository `exampleapp`:

```
mooseDocumentedError("exampleapp", 235, "Feature ABC is not currently supported.");
```

where this error is documented in `exampleapp`'s issue #235 at `github.com/someuser/exampleapp/235`.

Note that for errors in the MOOSE framework itself, the MOOSE repository is
registered with the name `moose` already, which occurs in the constructor
of `MooseApp`, from which every app should inherit.
