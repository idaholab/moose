# Combine CSV

MOOSE is able to output vector postprocessor comma separated value (CSV) files.
Also, other generated CSV files for input and output from MOOSE are created.
Combine CSV is a simple tool to combine CSV files into one file.
Combine CSV is able to output the combine CSV file in a bilinear format that
may be directly used by MOOSE (e.g., [PiecewiseBilinear.md]).

Input files require a specific format for names. The basename is followed by
a four digit code representing the timestep number and then ".csv" extension.
The time file replaces the four digit code with "time.csv". A simple example of
file names is:

```text
basename_pattern_0000.csv  basename_pattern_0001.csv  basename_pattern_0002.csv basename_pattern_time.csv
```

## Standard Usage

Combine CSV is a command line script, `combine_csv.py`. To display all command
line options with some help descriptions:

```text
combine_csv.py -h
```

The non-bilinear format of the output single CSV file has timestep number
(or time if a "*time.csv" file is provide and `-t` option is used) as each
column's header, if written with `-w`. Row values are ordered as they are
found in each input file. If the "X variable" name is provided after `-x` then
the first column will have these values.

## Bilinear Usage

Outputting the CSV file in bilinear format requires command line options `-b`
and `-w`. Also, usually `-t` is used where a "*time.csv" file defines what
time corresponds to timestep number. A typical command line execution is:

```text
combine_csv.py basename_pattern_ -o out_file -x x_name -y y_name -w -t -b
```

The bilinear output format transposes a standard output's data. The first
column header is also omitted during output.

## Error Messages

Error messages are put through Python's exception handling method. Initial
errors directly create specific errors from Combine CSV that have messages
identifying the error and what went wrong. These are usually produced from
wrong command line arguments.
