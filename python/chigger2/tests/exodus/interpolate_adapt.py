#!/usr/bin/env python
import chigger

# The file has two timesteps at 2 and 11, the data
# ranges betweein [0, 5] and [0, 14], respectively.
# Thus, at t=6.5 the range should be [0, 9.5].
filename = '../input/input_out.e'
reader = chigger.exodus.ExodusReader(filename)
result = chigger.exodus.ExodusResult(reader, lim=(0,14), variable='u')
result.setParams('edges', visible=True)
window = chigger.RenderWindow(result, size=(300, 300))

# time = 2, timestep = 0
reader.setParams(timestep=0)
print result.getRange() # [0, 5]
window.write('interpolate_adapt_2.png')

# time = 11, timestep = 1
reader.setParams(timestep=1)
print result.getRange() # [0, 14]
window.write('interpolate_adapt_11.png')

# time = 6.5, timestep = N/A (interpolate)
reader.setParams(time=6.5)
print result.getRange() # [0, 9.5]
window.write('interpolate_adapt_6-5.png') # time = 6.5

window.start()
