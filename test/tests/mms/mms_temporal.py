#!/usr/bin/env python
import mms
df1 = mms.run_temporal('mms_temporal.i', 4, console=False)
df2 = mms.run_temporal('mms_temporal.i', 4, 'Executioner/scheme=bdf2', console=False)

fig = mms.ConvergencePlot(xlabel='$\Delta t$', ylabel='$L_2$ Error')
fig.plot(df1, label='1st Order (Implicit Euler)', marker='o', markersize=8)
fig.plot(df2, label='2nd Order (Backward Difference)', marker='o', markersize=8)
fig.save('mms_temporal.pdf')

#TESTING (leave this comment, it is used in doco to remove the following from a demo)
df1.to_csv('mms_temporal_first.csv')
df2.to_csv('mms_temporal_second.csv')
