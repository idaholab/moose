# Otter the Plotter

Otter is a generic 2D line plot and data processing tool. It provides (chained)
data sources for reading and processing data, and outputs for writing new files
and or plots.

Data sources:

* `CSV` - reads comma separated value files with a header (like MOOSE postprocessor outputs)
* `Difference` - computes the difference between two data sources (uses linear interpolation to sample on a union of all x points)

Outputs

* `Plot` - uses [matplotlib](https://matplotlib.org) to write a plot of one or more data sources (as PDF or PNG)
* `WriteCSV` - writes a new CSV file from one or more data sources (uses linear interpolation to sample on a union of all x points)

Features:

* Dependency resolution between data sources
* HIT input file format with Factory system
