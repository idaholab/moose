#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

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
