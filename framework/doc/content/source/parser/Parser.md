# Parser

The MOOSE Parser object is responsible for interacting with the HIT input file
parser. It is important to note that +this+ object is not responsible for the raw
file-base I/O. The underlying structure of the MOOSE input file is dictated by
HIT and information on the format can be found
[here](/application_usage/input_syntax.md optional=True). The parser abstraction
expects information to be organized into a hierarchy of blocks with zero or more
children at each level. It performs as much error checking as early as possible before
building any MOOSE objects.

## Brace expressions

Field values may contain one or more `${...}` brace expressions that are expanded
before the value is handed to the rest of MOOSE; see
[Brace Expressions](input_syntax.md optional=True) for the user-facing syntax and the
list of built-in commands. Parsing and evaluation are implemented by
`hit::BraceExpander` in
[braceexpr.h](/framework/contrib/hit/include/hit/braceexpr.h) and
[braceexpr.cc](/framework/contrib/hit/src/hit/braceexpr.cc), and support brace
expressions nested to arbitrary depth, e.g.:

```text
${replace ${raw foo ${num}}}
${units ${fparse 2 * ${x}} m -> cm}
```

### Structure

A field value is parsed into a `hit::BraceTemplate`: an ordered sequence of parts that
are each either literal text or a `hit::BraceExpr` (a single `${...}` expression).
Each `BraceExpr` holds a list of whitespace-separated `hit::BraceArg`s, and each
`BraceArg` holds its own `BraceTemplate`, so an expression's argument may itself
contain literal text and further nested expressions. Representing both a whole field
value and a single command argument with the same `BraceTemplate` structure is what
allows expressions to nest arbitrarily. Parsing is done by a small hand-written
recursive-descent parser (`BraceTemplateParser` in `braceexpr.cc`); no external
parsing library is used. Each parsed `BraceExpr` records its source line, column,
offset, and length so evaluation errors can be reported against the exact `${...}`
span that produced them.

### Evaluation

`BraceExpander::evaluate` walks a `BraceTemplate` recursively and evaluates innermost
expressions first: every `BraceArg` of a `BraceExpr` is fully expanded to a string
before the expression's own command runs, so a nested expression is always resolved
before the expression that contains it, no matter how deeply it is nested.

Each command is implemented as a `hit::Evaler`:

```cpp
virtual EvalResult eval(Field * n, const std::vector<std::string> & args, BraceExpander & exp) = 0;
```

An evaler receives its already-expanded string arguments and returns an `EvalResult`
(a value, a `Field::Kind`, and the paths of any other fields it referenced) rather
than mutating the destination `Field` directly. If an expression has exactly one
argument, the `replace` command is dispatched implicitly (`${foo}` is shorthand for
`${replace foo}`); otherwise the first argument names the command and is looked up
among the evalers registered with `BraceExpander::registerEvaler`. The built-in
commands (`replace`, `raw`, `env`) are defined in `braceexpr.cc`; MOOSE registers its
own commands (`fparse`, `units`, `enumerate`, `repeat`) in `Parser.C`.

### Field resolution

`BraceExpander::resolve` resolves a field's value on demand rather than requiring
fields to be written in dependency order: resolving one field can trigger resolution
of another field it references (through `${replace ...}` or `${fparse ...}`), and
each field's result is cached the first time it is resolved. A per-field state
(`Unvisited`/`Resolving`/`Resolved`) detects cyclic dependencies and reports the full
chain of fields involved when a field's expansion depends on itself, directly or
indirectly.

### Type propagation

A resolved field keeps a `Field::Kind` alongside its string value. If the field's
value is exactly one top-level expression with no surrounding literal text, the field
inherits that expression's result kind, e.g. `value = ${fparse 1 + 2}` becomes a
float. Literal text around an expression, or more than one expression in the value,
forces `Field::Kind::String` instead, e.g. `value = foo_${bar}` and
`value = '${bla1} and ${bla2}'` both stay strings.

### Diagnostics

Source spans recorded while parsing are mapped back to the field's location in the
input file, so a malformed or failing expression is reported with the file, line, and
column of the specific `${...}` sub-expression rather than of the whole field.
Evalers report errors through `BraceExpander::currentExpressionErrorMessage`, which
anchors the message to whichever expression is currently being evaluated.

## Includes

See [Includes](input_syntax.md optional=True)

## Multiple inputs

When multiple inputs are supplied to a MOOSE application, the parser will read
them in successively and add merge them into a single block hierarchy. Later
inputs add to and override parameters to previous inputs. This permits the user
to factor out common parts of a set of inputs and reuse them in multiple
simulations.
