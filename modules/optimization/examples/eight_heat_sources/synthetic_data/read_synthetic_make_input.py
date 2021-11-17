#!/opt/moose/miniconda/bin/python
import pandas as pd
import math
import glob, os



df = pd.DataFrame()
all_files=glob.glob('synthetic_forward_out_ln[1 5 9]_0001.csv')
read_df=pd.concat((pd.read_csv(f) for f in all_files))
df = pd.concat([df,read_df],axis=1)
print('df names=', list(df.columns),'   df shape=',list(df.shape))

print("\n\n****************  master.i  ***************\n\n")

#    [OptimizationReporter]
#      type = ObjectiveGradientMinimize
#      adjoint_vpp = 'adjoint_results'
#      adjoint_data_computed = 'adjoint_rec_0 adjoint_rec_1 adjoint_rec_2'
#      parameter_vpp = 'parameter_results'
#      data_computed = 'data_rec_0 data_rec_1 data_rec_2 data_rec_3'
#      data_target = '
#    []

print("[OptimizationReporter]")
print("  type = ObjectiveGradientMinimize")
print("  adjoint_vpp = \'adjoint_results\'")
print("  adjoint_data_computed = \'ar00 ar01 ar02 ar03 ar04 ar05 ar06 ar07\'")
print("  parameter_vpp = \'parameter_results\'")

print("  data_computed = \'", end = '')
for i in range(len(df)):
    print("dr"+str(f'{i:02}'), end = ' ')
print("\'")

print("  data_target = \'", end = '')
for temp in df['temperature']:
    print("{0:.2f}".format(temp), end = ' ')
print("\'")
print("[]")

print("\n\n****************  forward.i  ***************\n\n")
#[Postprocessors]
#[dr00]
#  type = PointValue
#  variable = temperature
#  point = '0.3 0.3 0'
#[]
# .
# .
#[]

print("\n\n[Postprocessors]")
for i in range(len(df)):
    print("[dr"+str(f'{i:02}')+"]")
    print("  type = PointValue")
    print("  variable = temperature")
    print("  point = \'" \
            +str(df['x'].iloc[i]) + " " \
            +str(df['y'].iloc[i]) + " " \
            +str(df['z'].iloc[i]) \
            +"\'")
    print("[]")
print("[]")


print("\n\n****************  adjoint.i  ***************\n\n")
#[DiracKernels]
#[a00]
#  type = ConstantPointSource
#  variable = temperature
#  value = 10
#  point = '0.3 0.3'
#[]
# .
# .
#[]

print("[DiracKernels]")
for i in range(len(df)):
    print("[a"+str(f'{i:02}')+"]")
    print("  type = ConstantPointSource")
    print("  variable = temperature")
    print("  value = 7.5")
    print("  point = \'" \
            +str(df['x'].iloc[i]) + " "\
            +str(df['y'].iloc[i]) + " " \
            +str(df['z'].iloc[i]) \
            +"\'")
    print("[]")
print("[]")
