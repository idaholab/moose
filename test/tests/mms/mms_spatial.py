#!/usr/bin/env python
import mms
x, y = mms.run_spatial('mms_spatial.i', 4, console=False)
x2, y2 = mms.run_spatial('mms_spatial.i', 4, 'Mesh/second_order=true', 'Variables/u/order=SECOND',
                          console=False)

fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(x, y, label='1st Order', marker='o', markersize=8)
fig.plot(x2, y2, label='2nd Order', marker='o', markersize=8)
fig.save('mms_spatial.pdf')
