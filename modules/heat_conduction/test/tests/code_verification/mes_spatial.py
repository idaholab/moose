#!/usr/bin/env python3

import mms
import os

def run_each_test(testName, hyperConvergent=False, refinementNo=5):

    # Look for an executable
    if os.path.isfile('../../../heat_conduction-opt'):
        executable = '../../../heat_conduction-opt'
    else:
        executable = '../../../../combined/combined-opt'

    df1 = mms.run_spatial(testName+'.i', refinementNo, console=False, executable=executable)

    if(not hyperConvergent):
        df2 = mms.run_spatial(testName+'.i', refinementNo, 'Mesh/second_order=true', 'Variables/u/order=SECOND',
                              console=False, executable=executable)

    fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
    fig.plot(df1, label=r'EDGE2 (1$^{st}$-order) $\hat{p}_O$', marker='o')
    if(not hyperConvergent): fig.plot(df2, label='EDGE3 (2$^{nd}$-order) $\hat{p}_O$', marker='D')
    fig.save(testName+'_spatial.png')

    df1.to_csv(testName+'_first_order.csv')
    if(not hyperConvergent): df2.to_csv(testName+'_second_order.csv')


''' 1D steady-state problems in Cartesian coordinates'''
run_each_test('cartesian_test_no1',True)
run_each_test('cartesian_test_no2')
run_each_test('cartesian_test_no3')
run_each_test('cartesian_test_no4',True)
run_each_test('cartesian_test_no5')

''' 1D steady-state problems in cylindrical coordinates'''
run_each_test('cylindrical_test_no1')
run_each_test('cylindrical_test_no2')
run_each_test('cylindrical_test_no3')
run_each_test('cylindrical_test_no4')
run_each_test('cylindrical_test_no5')

''' 1D steady-state problems in spherical coordinates'''
run_each_test('spherical_test_no1')
run_each_test('spherical_test_no2')
run_each_test('spherical_test_no3')
run_each_test('spherical_test_no4')
run_each_test('spherical_test_no5')
