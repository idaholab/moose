#!/usr/bin/env python
import mms
x, y = mms.run_temporal('mms_temporal.i', 4, console=False)
x2, y2 = mms.run_temporal('mms_temporal.i', 4, 'Executioner/scheme=bdf2', console=False)

fig = mms.ConvergencePlot(xlabel='$\Delta t$', ylabel='$L_2$ Error')
fig.plot(x, y, label='Implicit Euler', marker='o', markersize=8)
fig.plot(x2, y2, label='2nd Order Backward Difference', marker='o', markersize=8)
fig.save('mms_temporal.pdf')
