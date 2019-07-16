# CommandLine

The CommandLine object is responsible for parsing and encapsulating the arguments
passed to the application via the command line. Normally developers and users
will +not+ interact with this object directly. Instead, arguments that appear
on the command line are normally spliced into the HIT parse tree when appropriate,
or are used to set [InputParameters.md] on the Application (MooseApp-derived) object
itself.

## Overriding Input File Syntax

When running multiple cases with similar input parameters, it can be convenient
to supply some arguments right on the command line. This is accomplished by
providing a fully-qualified name/value pair on the command line corresponding
to the parameter you wish to override or add.

For example given this snippet of an input file either one or both of these
parameters could be overridden from the command line with the syntax shown.
Entirely new sections can be constructed right on the command line if desired.

```
[Section]
  [object]
     some_parameter = foo
     another_parameter = bar
  []
[]


# On the command line
./MyApp-opt -i input.i Section/object/some_parameter=new_value
./MyApp-opt -i input.i Section/object/new_parameter=some_value
```

## Overriding MultiApp Syntax

Syntax from MultiApp files can be overridden as well. Since a single input file
can and is often used to spawn several SubApps a custom syntax was developed
to allow users to override one or more MultiApps or individual SubApps at a time.

Given the same input file syntax seen above see the syntax below for the various
ways in which SubApp syntax can be overridden or supplied on the command line.

```
[Multiapp]
  [sub_app_left]
    ...
  []

  [sub_app_right]
    ...
  []
[]

# Global MultApp Override
./MyApp-opt -i input.i :Section/object/some_parameter=new_value

# Whole MultiApp Override
./MyApp-opt -i input.i sub_app_left:Section/object/some_parameter=new_value

# Individual SubApp Override
./MyApp-opt -i input.i sub_app_left0:Section/object/some_parameter=new_value
                       sub_app_left1:Section/object/some_parameter=different_value
```
