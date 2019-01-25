# CSVDiff Tool

The supplied CSVDiff tool (csvdiff.py) provides the TestHarness the capability to perform differentiations with comma separated value (CSV) files.

## Basic Usage

In it's simplest behavior, performing a differentiation on two CSV files (a and b) requires the following syntax:

```
csvdiff.py a b
```

If the two files are the same, the program will state that it is so, and exit with return code 0. Example:

```
> echo -e "x\n0" | tee a b
> moose/scripts/csvdiff.py a b
Files are the same

> echo $?
0
```

A detected difference will be stated, and exit with a non-zero error code. Example:

```
> echo -e "x\n0.000001" > a
> echo -e "x\n0.000001000005501" > b

> moose/scripts/csvdiff.py a b
In file b: The values in column "x" don't match @ t0
    relative diff:   0.000001 ~ 0.000001 = 0.000006 (5.501e-06)

> echo $?
1
```

## Extended Usage

The CSVDiff tool can be used to test specific fields with specific error tolerances. It can also be made to detect when not to perform a differentiation
if the value being tested is below a certain threshold (floor, or zero). These features can be used as direct arguments to csvdiff.py, or through the use
of a comparison file.

## Syntax

```
./csvdiff.py CSV_FILE CSV_FILE [additional arguments]
```

!alert note
Always specify the two files you wish to perform a differentiation on, before any other options

| Arguments | Value | Help |
| :- | :- | :- |
| `--summary or -s` | *csv_file* | Create a comparison file based on *csv file* |
| `--comparison-file or -c` | *comparison file* | Use specified comparison file while performing differentiations |
| `--ignore-fields` | *str* | A list of space separated fields to ignore when performing differentiations |
| `--diff-fields` | *str* | A list of space separated fields to include when performing differentiations |
| `--abs-zero` | *str float* | A scientific notiation or float value representing zero (the floor). Any values lower than this amount will be considered zero. (default: 1e-11) |
| `--relative-tolerance` | *str float* | A float or scientific notation value representing an acceptable degree of tolerance between two opposing values. Any float comparison which falls within this tolerance will be considered the same number. (default 5.5e-6) |
| `--custom-columns` | *str* | Space separated list of custom field IDs to compare |
| `--custom-abs-zero` | *str float* | Space separated list of scientific notations or floats for absolute zero, corresponding to the values in --custom-colums |
| `--custom-rel-err` | *str float* | Space separated list of scientific notations or floats for relative tolerance, corresponding to the values in --custom-colums |

## Comparison File

Using a comparison file is ideal when needing to adjust a complex set of fields and tolerances, which would make for a very long and confusing command line argument. The CSVDiff tool can generate this comparison
file which, can be used to set the above arguments quickly.

To generate a comparison file, run the CSVdiff tool with the appropriate `--summary csv_file` argument. In the following example, we use `echo` to create a simple csv file. We then instruct csvdiff.py to create
a comparison file from our csv file, and redirect the output to a file named `a.cmp`:

```
> echo -e "x\n0" > a
> moose/scripts/csvdiff.py --sumary a > a.cmp
> cat a.cmp
TIME STEPS relative 1 floor 0  # min: 0 @ t0  max: 0 @ t0

GLOBAL VARIABLES relative 5.5e-06 floor 1e-11
    x                    # min: 0.000e+00 @ t0          max: 0.000e+00 @ t0
```

You can then edit this file and modify key sections to control tolerances, or instruct csvdiff to ignore an entire field all together.

The 'TIME STEPS' field is a special header, which currently is not used and is present for future capabilities yet to be added to the CSVDiff tool.

The 'GLOBAL VARIABLES' field allows you to change the tolerance for every field present in the CSV file. There are two key parameters; relative and floor. You can modify one or both of the
values immediately following the parameter to suite your needs. You can also modify the tolerances for each individual field. In the case of our example, 'x' is the only field in our CSV
file. To adjust only that field's tolerance values, we can add a parameter directly proceeding the 'x' label:

```
TIME STEPS relative 1 floor 0  # min: 0 @ t0  max: 0 @ t0

GLOBAL VARIABLES relative 5.5e-06 floor 1e-11
    x relative 5.5e-06   # min: 0.000e+00 @ t0          max: 0.000e+00 @ t0
```

The above change does nothing, as the relative error value we added is the same as the global relative error value. You can also add both relative and floor tolerances to this line. As well as comments and other logical statements:

```
GLOBAL VARIABLES relative 5.5e-06 floor 1e-11
    # loosen tolerances for x
    x floor 1e-8 relative 2e-04

    # do not run differential tests for field y
    !y

    z
```

Here we added comments, loosened both the floor and error tolerances for field 'x'. Field 'y' will be ignored entirely. The 'z' field we left alone, and will end up using the global values set forth by the global variables header line.

## A Real Example

Consider the following two CSV files:

File a:

```
x,y,z
0,0,100
1,0.000001,1
```

File b:

```
y,z,x
0,100,0
0.000001000005501,1,1
```

!alert note
We purposely altered the field header to demonstrate CSVDiff's capability of correctly mapping the field labels between two files.

If we run csvdiff.py on a and b, we see there is a small difference of 5.501e-06 for field 'y' at time step 1 (or simply put, row 1). Just a bit more than what our default global tolerance allows for:

```
> moose/scripts/csvdiff.py a b
In file b: The values in column "y" don't match @ t1
    relative diff:   0.000001 ~ 0.000001 = 0.000006 (5.501e-06)
```

We can create a comparison file to set forth new tolerances which will allow the two files to be considered identical. Start off by creating a comparison file using file 'a':

```
> moose/scripts/csvdiff.py --summary a > a.cmp
```

The following example changes would allow both files to be considered identical:

Loosen the error tolerances for 'y':

```
TIME STEPS relative 1 floor 0  # min: 0 @ t0  max: 1 @ t1

GLOBAL VARIABLES relative 5.5e-06 floor 1e-11
    y relative 5.501e-06 # min: 0.000e+00 @ t0          max: 1.000e-06 @ t1
    x                    # min: 0.000e+00 @ t0          max: 1.000e+00 @ t1
    z                    # min: 1.000e+00 @ t1          max: 1.000e+02 @ t0
```

Raise the floor tolerance:

```
TIME STEPS relative 1 floor 0  # min: 0 @ t0  max: 1 @ t1

GLOBAL VARIABLES relative 5.5e-06 floor 1e-11
    y floor 1e-5         # min: 0.000e+00 @ t0          max: 1.000e-06 @ t1
    x                    # min: 0.000e+00 @ t0          max: 1.000e+00 @ t1
    z                    # min: 1.000e+00 @ t1          max: 1.000e+02 @ t0
```

Ignore field 'y' by including a not '!' statement:

```
TIME STEPS relative 1 floor 0  # min: 0 @ t0  max: 1 @ t1

GLOBAL VARIABLES relative 5.5e-06 floor 1e-11
    !y                   # min: 0.000e+00 @ t0          max: 1.000e-06 @ t1
    x                    # min: 0.000e+00 @ t0          max: 1.000e+00 @ t1
    z                    # min: 1.000e+00 @ t1          max: 1.000e+02 @ t0
```

Removing the offending field from the comparison file:

```
TIME STEPS relative 1 floor 0  # min: 0 @ t0  max: 1 @ t1

GLOBAL VARIABLES relative 5.5e-06 floor 1e-11
    x                    # min: 0.000e+00 @ t0          max: 1.000e+00 @ t1
    z                    # min: 1.000e+00 @ t1          max: 1.000e+02 @ t0
```

Any one of the above example comparison files, would allow a and b to be considered identical:

```
> moose/scripts/csvdiff.py a b --comparison-file a.cmp
Files are the same
```

!alert note title=Exodiff-like summary report
The summary report follows the same output style as another popular tool: `exodiff -summary`. By design, the two summary reports are interchangeable.
