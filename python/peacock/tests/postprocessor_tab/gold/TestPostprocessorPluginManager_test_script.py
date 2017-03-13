"""
python TestPostprocessorPluginManager_test_script.py
"""
import matplotlib.pyplot as plt
import mooseutils

# Create Figure and Axes
figure = plt.figure(facecolor='white')
axes0 = figure.add_subplot(111)
axes1 = axes0.twinx()

# Read Postprocessor Data
data = mooseutils.PostprocessorReader('../input/white_elephant_jan_2016.csv')
x = data('time')

y = data('air_temp_set_1')
axes1.plot(x, y, marker='', linewidth=5.0, color=[0.2, 0.627, 0.173, 1.0], markersize=1, linestyle=u'--', label='air_temp_set_1')

# Axes Settings
axes1.legend(loc='lower right')
axes0.set_title('Snow Data')

# y1-axis Settings
axes1.set_ylabel('Air Temperature [C]')
axes1.set_ylim([0.0, 35.939999999999998])

# Show figure and write pdf
plt.show()
figure.savefig("output.pdf")
