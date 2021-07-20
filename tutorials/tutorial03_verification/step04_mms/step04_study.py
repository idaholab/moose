#!/usr/bin/env python3

# MooseDocs:start:spatial
import mms

df1 = mms.run_spatial(['2d_main.i', '2d_mms_spatial.i'], 4, console=False)
df2 = mms.run_spatial(['2d_main.i', '2d_mms_spatial.i'], 4, 'Mesh/second_order=true', 'Variables/T/order=SECOND', console=False)

fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df1, label='1st Order', marker='o', markersize=8)
fig.plot(df2, label='2nd Order', marker='o', markersize=8)
fig.save('2d_mms_spatial.png')
# MooseDocs:end:spatial

# MooseDocs:start:temporal
import mms

df1 = mms.run_temporal(['2d_main.i', '2d_mms_temporal.i'], 4, dt=1200, mpi=4, console=False)
df2 = mms.run_temporal(['2d_main.i', '2d_mms_temporal.i'], 4, 'Executioner/scheme=bdf2', dt=1200, mpi=4, console=False)
fig = mms.ConvergencePlot(xlabel='$\Delta t$', ylabel='$L_2$ Error')
fig.plot(df1, label='1st Order', marker='o', markersize=8)
fig.plot(df2, label='2nd Order', marker='o', markersize=8)
fig.save('2d_mms_temporal.png')
# MooseDocs:end:temporal
