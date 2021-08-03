#!/usr/bin/env python3

# MooseDocs:start:spatial
import mms
df = mms.run_spatial('1d_analytical.i', 6, 'Mesh/gen/nx=10', console=False)
fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df, label='1st Order', marker='o', markersize=8)
fig.save('1d_analytical_spatial.png')
# MooseDocs:end:spatial

# MooseDocs:start:temporal
import mms
df = mms.run_temporal('1d_analytical.i', 6, dt=0.5, console=False)
fig = mms.ConvergencePlot(xlabel='$\Delta t$', ylabel='$L_2$ Error')
fig.plot(df, label='2nd Order (Backward Difference)', marker='o', markersize=8)
fig.save('1d_analytical_temporal.png')
# MooseDocs:end:temporal
