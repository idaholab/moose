
# Input File Syntax

MOOSE input file syntax basically works like this:

```
# comment
[section] # inline-comment
  field01 = 'quoted-string'
  field02 = "quoted-string"
  field03 = "multi-line"
            "string"
  field04 = unquoted_string # can't have whitespace

  field05 = 42 # integer
  field06 = 42.42 # floating point number
  field07 = true # boolean (false, on, off - case insensitive)

  field08 = 'item0 item1 item2' # array of items (strings or numbers)
  field09 = 'item00 item01 ;
             item10 item11 ;
             item20' # double indexed array (can even be jagged)
  field10 = 'item000 item001 ;
             item010 ;
             item020 item021 item022 |
             item100 item101 item102 ; ; # can use empty sub-sub-vectors
             item120 | | # can also use empty sub-vectors
             item300 item301 ;
             item310 item311' # triple indexed array (primary and secondary delimiters are '|' and ';')

  [subsection]
    foo = 42
  [] # close subsection

  [another_subsection]
  []
[] # close section [../] is also allowed, but deprecated

[another_section]
[]
...
```

Note that single `'` and double `"` quotes can be used interchangeably to quote strings (as long
as the start quote and end quote character are the same) and are functionally equivalent. They
do however behave differently when auto-formatting input files with [`hit format`](hit.md).
Single quotes strings will not be reformatted, while double quoted strings are reindented
and reflowed.

## Brace Expressions

Brace expressions allow you to assign computed/calculated values to fields in your input files.
Brace expressions have the following syntax:

```
${cmd [arg1] [arg2]...}
```

where arguments are whitespace delimited and are allowed to be brace expressions themselves.
Brace expressions can appear as field values either unquoted or quoted (i.e. inside a
string-value).  MOOSE currently has five built-in brace-expression commands:

- `${replace <var-name>}`: looks up the parameter `var-name` in the input file searching from the
  current scope looking at consecutive parent scopes until one exists, substituting its value
  for the replacement expression.

- `${raw <arg>...}`: concatenates all arguments' values with no intervening spaces.

- `${env <var-name>}`: evaluates to the named environment variable.

- `${fparse <arg>...}`: takes a
  [Function Parser math expression](http://warp.povusers.org/FunctionParser/fparser.html#literals)
  that can reference fields from the current input file and evaluates it with the function
  parser. Variables referencing input-file fields are looked up using the same procedure as with
  the `replace` command.

- `${units <arg> <unit> [-> <to_unit>]}`: takes an argument `arg` in physical units
  of `unit` and converts it to the unit `to_unit` using the [MooseUnit](/utils/Units.md)
  system. For example the expression `${units 1 J/mol -> eV/at}` would evaluate to the value
  `1.0364269656262e-05`. The `to_unit` argument is optional resulting in a no-op that serves
  only for documentation purposes (`${units 1 J/mol}` would evaluate to the value `1.0`).

Examples:

```
foo1 = 42
foo2 = 43
[section1]
  num = 1
  bar = ${replace ${raw foo ${num}}}  # becomes 42
  bar2 = ${${raw foo ${num}}} # shortcut for above expression - becomes 42
[]
[section2]
  num = 2
  bar = ${${raw foo ${num}}} # becomes 43
[]

a = ${fparse
      ${section1/bar} + foo1 / foo2
     } # becomes 42+42/43 or 42.976744...
```

Here are some important details about how brace-expressions are evaluated:

- Brace expressions are evaluated in input-file order (i.e. lexical order) from top to bottom of
  the input file.  Brace expressions must *not* depend on field values using brace expressions
  that occur later in the input file - although they can depend on field values without brace
  expressions that occur later in the input file.

- Nested brace expressions are evaluated from the most deeply nested outward.

- A single field value is allowed to have multiple brace expressions if (and only if) the field
  value is presented as a quoted string - e.g. `foo = '${bla1} and ${bla2}'`. These are
  evaluated from left to right.

- If there are no arguments in the brace-expression beyond the "cmd" (e.g. `${foo}`), then the
  `replace` command is implied: e.g. `${foo}` means `${replace foo}`.

## Overriding input parameters from the command line

See the [CommandLine.md] object for information on how input parameters can be
changed on the command line.

## Includes

Other input files may be included using the following syntax:

```
!include path/to/input.i
```

This can be used in any arbitrary nested context in an input file, and included files
can include other files. The only requirement is that the included files
must contain a set of syntactically complete blocks or parameters.

Functionally, including a file is equivalent to inserting the
text of the file at the `!include` location. Therefore,
for example, if the included file uses a value `${somevar}` defined in the parent file
via `somevar = somval`, then the `!include` statement must occur *after* the
`somevar` definition.

!alert! warning title=Beware merge-related include ordering issues
When an included file contains the same block as the parent file, the blocks get
*merged*. The merge moves the second instance of the block into the first, not
the first into the second. For example,

`file1.i`:

```
[BlockA]
  param1 = 4
[]

!include file2.i
```

`file2.i`:

```
val3 = 8

[BlockA]
  param2 = ${val3}
[]
```

This effectively produces the input

```
[BlockA]
  param1 = 4
[]

val3 = 8

[BlockA]
  param2 = ${val3}
[]
```

and after the merge operation,

```
[BlockA]
  param1 = 4
  param2 = ${val3}
[]

val3 = 8
```

This results in an error, since `${val3}` is now used before it is defined.
Therefore, files must be partitioned with this behavior in mind.
!alert-end!

!alert warning title=Included input parameters cannot be overridden by default
Note that parameters from the parent or included files do not override each other by default,
as this is just interpreted as providing the input parameter twice, resulting in a parsing error.
To override parameters, you must either use command-line arguments or the explicit
override syntax defined below.

## Parameter override syntax

Ordinarily, redefining a parameter results in an input error:

```
param1 = 3
param1 = 4   # error due to duplicate parameter
```

However, there is a syntax for explicitly specifying that you would like to
allow overriding a previously defined value, using either `:=` or `:override=`
instead of `=`:

```
param1 = 3
param1 := 4           # not an error; now param1 is 4 instead of 3
param1 :override= 5   # now param1 is 5
```

Note that `:=` and `:override=` are exactly the same; the latter is provided for
those wanting to make the syntax more visible to a reader of the input file.

As noted with the `!include` syntax above, redefining a parameter with the
`=` assignment operator results in an duplicate parameter error; the explicit
override syntax can be used to fix this:

`myinput.i`:

```
!include base.i

[BlockA]
  param1 := new_value   # using "=" would result in an error
[]
```

`base.i`:

```
[BlockA]
  param1 = original_value
[]
```
