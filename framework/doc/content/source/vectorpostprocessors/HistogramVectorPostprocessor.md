# HistogramVectorPostprocessor

## Short Description

!syntax description /VectorPostprocessors/HistogramVectorPostprocessor

## Description

Used to compute the histogram for all columns of another VectorPostprocessor (VPP).  The only inputs are the other VPP (`vpp`) and the number of bins to use in the histogram (`num_bins`).

This will actually generate three columns for each column in the original VPP. The `column_name` is the name of the vector considered in the histogram and is also the
name used to declare the vector containing the histogram data.

- column_name: The histogram data for the vector in the original VPP
- column_name_lower: The lower bound for each bin
- column_name_upper: The upper bound for each bin


## Plotting

MOOSE comes with built-in plotting capabilities that can help plot the output of a HistogramVectorPostprocessor.  These plotting capabilities are part of the `Chigger` suite of visualization tools located in the `moose/python` directory.  To use them you must add the full path of your `moose/python` directory to the environment variable called `$PYTHONPATH` using something like:

```bash
export PYTHONPATH=/full/path/to/moose/python:$PYTHONPATH
```

Once that is completed a script such as the following will plot your data:

```python
import matplotlib.pyplot as plt
import mooseutils

# Create Figure and Axes
figure = plt.figure(facecolor='white')
axes0 = figure.add_subplot(111)

# Read Postprocessor Data
data = mooseutils.PostprocessorReader('histogram_vector_postprocessor_out_histo_0001.csv')

# Grab upper and lower bin bounds
lower = data('value_lower')
upper = data('value_upper')

# Compute the midpoint and width of each bin
mid = (lower + upper) / 2.0
width = upper - lower

# Grab the data to be plotted
y = data('value')

# Plot everything
axes0.bar(mid, y, width=width)

# Show the plot and save it
plt.show()
figure.savefig("output.pdf")
```

!syntax parameters /VectorPostprocessors/HistogramVectorPostprocessor

!syntax inputs /VectorPostprocessors/HistogramVectorPostprocessor

!syntax children /VectorPostprocessors/HistogramVectorPostprocessor
