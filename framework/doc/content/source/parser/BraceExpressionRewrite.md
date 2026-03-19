# Brace Expression Rewrite

## Goal

Rewrite the HIT brace-expression system so that:

- different brace-expression commands can nest arbitrarily;
- nested expressions are evaluated strictly from the innermost expression outward;
- commands operate on parsed structure rather than flattened strings;
- field resolution, type propagation, and diagnostics are predictable.

This note is an implementation sketch for replacing the current `hit::BraceExpander`
design in [framework/contrib/hit/include/hit/braceexpr.h](/framework/contrib/hit/include/hit/braceexpr.h)
and [framework/contrib/hit/src/hit/braceexpr.cc](/framework/contrib/hit/src/hit/braceexpr.cc).

## Current Problems

The current implementation has three structural weaknesses:

1. It does not build a real syntax tree for brace expressions. `BraceNode` only
   stores either a raw token string or a list of children.
2. It tokenizes arguments by whitespace, so command arguments lose structure
   before the evaler sees them.
3. Evalers mutate the destination `Field` while evaluating nested content,
   which mixes parsing, evaluation, and result typing.

That design is good enough for simple expressions such as `${foo}` or
`${units 1 km -> m}`, but it is fragile for nested mixed-command forms such as:

```text
${replace ${raw foo ${num}}}
${units ${fparse 2 * ${x}} m -> cm}
${replace prefix_${raw foo ${num}}}
${${raw foo ${num}}}
```

The documentation already promises innermost-first nesting semantics, so the
rewrite should make that behavior explicit and enforceable.

## Proposed Architecture

Split the subsystem into four layers:

1. `BraceParser`
   Parses a field value into an AST using PEGlib.
2. `BraceEvaluator`
   Recursively evaluates the AST from children to parent.
3. `FieldResolver`
   Resolves references to other fields, with caching and cycle detection.
4. `BraceExpansionWalker`
   Iterates over HIT fields and applies the resolver to each field value.

The parser should only understand the brace-expression language embedded in a
single field value. HIT parsing remains unchanged.

## AST

Replace `BraceNode` with explicit node kinds. A minimal model is:

```cpp
namespace hit
{

struct SourceSpan
{
  std::size_t begin = 0;
  std::size_t end = 0;
};

struct TemplateNode;
struct ExprNode;

using TemplatePart = std::variant<std::string, std::unique_ptr<ExprNode>>;

struct TemplateNode
{
  SourceSpan span;
  std::vector<TemplatePart> parts;
};

struct ArgNode
{
  SourceSpan span;
  TemplateNode value;
};

struct ExprNode
{
  SourceSpan span;
  std::vector<ArgNode> args;
};

} // namespace hit
```

Important properties:

- `TemplateNode` represents any string that may contain literal text and nested
  `${...}` expressions.
- `ExprNode` stores arguments as `ArgNode`, not as flattened strings.
- `ArgNode` stores a `TemplateNode`, so nested expressions can appear inside an
  argument token.
- Spans are tracked for error reporting.

This is enough to represent both a whole field value and individual command
arguments with the same structure.

## PEGlib Grammar

The brace-expression parser should use PEGlib. The grammar only needs to parse
content inside a single field value:

```peg
Template        <- Part*
Part            <- Expr / Literal
Expr            <- '${' WS* ExprBody? WS* '}'
ExprBody        <- Arg (WS+ Arg)*
Arg             <- ArgPart+
ArgPart         <- Expr / ArgText
Literal         <- (!'${' .)+
ArgText         <- (!WS !'}' !'${' .)+
WS              <- [ \t\r\n]
```

Notes:

- `Template` is used for the whole field value.
- `Expr` is recursive, so nesting depth is unbounded.
- `ArgText` stops at whitespace, `}`, or `${`, which preserves existing command
  tokenization rules while still permitting nested expressions inside an
  argument.
- `Literal` is only used outside an expression body, allowing quoted strings
  like `'foo ${bar} baz'`.

The parser should expose:

```cpp
TemplateNode parseBraceTemplate(const std::string & input);
```

It should throw `hit::Error` with precise spans for malformed expressions such
as an unmatched `}` or `${`.

## Evaluator Contract

Replace the current evaler signature:

```cpp
virtual std::string eval(Field * n,
                         const std::list<std::string> & args,
                         BraceExpander & exp) = 0;
```

with a structured contract:

```cpp
namespace hit
{

enum class BraceValueKind
{
  Inherit,
  String,
  Int,
  Float,
  Bool
};

struct EvalResult
{
  std::string text;
  BraceValueKind kind = BraceValueKind::Inherit;
  std::vector<std::string> used_paths;
};

struct EvaluatedArg
{
  const ArgNode * node;
  EvalResult value;
};

class EvalContext
{
public:
  virtual ~EvalContext() = default;

  virtual EvalResult resolveField(Field & dst, const std::string & path) = 0;
  virtual void addError(const std::string & message, const SourceSpan & span) = 0;
  virtual const std::string & fieldText() const = 0;
};

class Evaler
{
public:
  virtual ~Evaler() = default;
  virtual EvalResult eval(Field & dst,
                          const ExprNode & expr,
                          const std::vector<EvaluatedArg> & args,
                          EvalContext & ctx) = 0;
};

} // namespace hit
```

The key change is that commands no longer mutate the `Field` directly. They
return a value plus an optional type. This removes the current side effects in
`ReplaceEvaler`, `FuncParseEvaler`, and `UnitsConversionEvaler`.

## Evaluation Algorithm

Evaluate with a recursive post-order traversal:

1. Parse the field value into `TemplateNode`.
2. Evaluate every nested `ExprNode` inside each `TemplateNode` before
   evaluating the parent node.
3. For each `ExprNode`:
   1. evaluate each `ArgNode` to an `EvalResult`;
   2. if there is one argument, dispatch implicit `replace`;
   3. otherwise evaluate the first argument as the command name and dispatch to
      the registered evaler;
   4. merge `used_paths` from all child results into the command result.
4. After the whole field template is resolved, apply `Field::setVal` once.

Pseudo-code:

```cpp
EvalResult evaluateTemplate(Field & field, const TemplateNode & node, EvalContext & ctx)
{
  EvalResult result;
  for (const auto & part : node.parts)
  {
    if (std::holds_alternative<std::string>(part))
      result.text += std::get<std::string>(part);
    else
    {
      auto child = evaluateExpr(field, *std::get<std::unique_ptr<ExprNode>>(part), ctx);
      result.text += child.text;
      appendUsed(result.used_paths, child.used_paths);
    }
  }
  return result;
}

EvalResult evaluateExpr(Field & field, const ExprNode & expr, EvalContext & ctx)
{
  std::vector<EvaluatedArg> args;
  for (const auto & arg : expr.args)
    args.push_back({&arg, evaluateTemplate(field, arg.value, ctx)});

  return dispatch(field, expr, args, ctx);
}
```

This makes innermost-first evaluation a natural consequence of the tree walk.

## Command Semantics

The built-in commands can keep their current user-facing syntax, but their
implementation should move to structured evaluation:

### `replace`

- Expect exactly one evaluated argument.
- Use `FieldResolver` to resolve the named field.
- Inherit the resolved field kind if the whole destination field is a single
  expression.

### `raw`

- Concatenate evaluated arguments with no separator.
- Result kind is `String`.

### `env`

- Expect exactly one evaluated argument.
- Read the environment variable named by the argument text.
- Result kind is `String`.

### `fparse`

- Concatenate evaluated arguments into the function expression text.
- Parse with `FunctionParser`.
- Resolve variable names using the same path lookup logic as `replace`.
- Return `Float`.

### `units`

- Preserve current syntax:
  `${units number from_unit -> to_unit}`
  `${units number unit}`
- Since each argument is already recursively evaluated, nested forms work
  without special cases.
- Return `Float`.

## Field Resolution

The current walker evaluates fields in lexical order. That behavior is enforced
by `Walker::traversalOrder()` defaulting to `BeforeChildren`, and by the parser
expanding every field during a tree walk.

That should be replaced with an explicit resolver:

```cpp
class FieldResolver : public EvalContext
{
public:
  EvalResult resolve(Field & field);
  EvalResult resolveField(Field & dst, const std::string & path) override;

private:
  enum class State
  {
    Unvisited,
    Resolving,
    Resolved
  };

  struct CacheEntry
  {
    State state = State::Unvisited;
    EvalResult result;
  };

  std::unordered_map<const Field *, CacheEntry> _cache;
};
```

Recommended behavior:

- If a field reference points to another brace-expression field, resolve that
  field on demand.
- Cache successful resolutions.
- Detect cycles using the `Resolving` state and report the dependency chain.

This is a stronger model than the current lexical-order restriction. If strict
backward compatibility is needed during rollout, this can be staged:

1. preserve current lexical-order field dependencies while replacing only the
   parser and evaluator internals;
2. add on-demand dependency resolution and cycle detection in a follow-up step.

## Type Propagation

The current code mutates `Field::kind()` mid-evaluation, then tries to repair it
later. The rewrite should use one rule:

- If the resolved field value is exactly one top-level `ExprNode` and no
  surrounding literal text exists, use the command result kind.
- Otherwise force `Field::Kind::String`.

That matches user expectations:

- `value = ${fparse 1 + 2}` becomes float.
- `value = '${fparse 1 + 2}'` stays string.
- `value = foo_${bar}` stays string.

## Diagnostics

Spans should be preserved from parsing through evaluation.

When a command fails, map the local span inside the field value back to the HIT
field location and emit a `hit::ErrorMessage` for the specific nested
expression. This is better than attaching every error to the whole field.

Examples that should improve:

- bad nested `fparse` syntax;
- invalid unit conversion operator placement;
- unknown command after nested command-name generation;
- cyclic field dependencies.

## Suggested File Layout

The rewrite can stay inside `framework/contrib/hit` with minimal parser-side
changes:

- `framework/contrib/hit/include/hit/braceexpr.h`
  new AST declarations, parser entry points, evaler API.
- `framework/contrib/hit/src/hit/braceexpr.cc`
  PEGlib parser, evaluator, built-in commands, field resolver.
- `framework/src/parser/Parser.C`
  registration of MOOSE-specific evalers only.

If the implementation grows, splitting is reasonable:

- `braceexpr_ast.h`
- `braceexpr_parse.cc`
- `braceexpr_eval.cc`

but keeping the external header stable is preferable.

## Migration Plan

### Phase 1: Add parser and AST

- Introduce AST types and PEGlib parser alongside the existing code.
- Add unit-style tests for parsing nested brace expressions.

### Phase 2: Add structured evaluator

- Implement `EvalResult`, `EvalContext`, and the new evaler interface.
- Port built-in `replace`, `raw`, and `env`.

### Phase 3: Port MOOSE evalers

- Port `FuncParseEvaler`.
- Port `UnitsConversionEvaler`.
- Remove direct `Field` mutation from evalers.

### Phase 4: Switch expansion driver

- Replace the current `BraceExpander::walk` implementation with AST-based
  evaluation.
- Preserve current output for simple existing inputs.

### Phase 5: Upgrade dependency resolution

- Add field-resolution caching and cycle detection.
- Decide whether to retain lexical-order restrictions or fully support
  on-demand resolution of referenced fields.

### Phase 6: Remove legacy code

- Delete `BraceNode`, `parseBraceNode`, and whitespace-token parsing helpers.

## Test Matrix

The existing parser substitution tests should be extended with focused coverage:

- nested same-command forms;
- nested mixed-command forms;
- nested generated command names;
- nested expressions inside quoted strings;
- nested expressions inside a command argument token;
- kind propagation for pure-expression vs mixed-template fields;
- invalid nesting syntax;
- invalid command names after nesting;
- unit conversion with nested numeric expressions;
- `fparse` with nested substitutions;
- cycle detection between fields;
- cross-field dependency resolution order.

Representative regression inputs:

```text
foo1 = 41
foo2 = 42
num = 1
bar = ${replace ${raw foo ${num}}}
calc = ${fparse ${bar} + ${units 1 m -> cm}}
name = ${${raw re place}}
mixed = 'a=${bar}; b=${fparse ${foo2} + 1}'
token = ${replace prefix_${raw foo ${num}}}
```

## Recommendation

The most important design decision is to stop passing flattened strings into
evalers. Once brace expressions are represented as an AST and evaluated with a
post-order traversal, arbitrary nesting becomes straightforward and the rest of
the behavior becomes much easier to reason about.
