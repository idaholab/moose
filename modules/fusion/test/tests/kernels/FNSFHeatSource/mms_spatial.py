import mms


df1 = mms.run_spatial('mms.i', 4, console=False, executable='../../../../fusion-opt')
df2 = mms.run_spatial('mms.i', 4, 'Mesh/second_order=true', 'Variables/temp/order=SECOND',
                      console=False, executable='../../../../fusion-opt')

fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df1, label='1st Order', marker='o', markersize=8)
fig.plot(df2, label='2nd Order', marker='o', markersize=8)
fig.save('mms_spatial.png')
