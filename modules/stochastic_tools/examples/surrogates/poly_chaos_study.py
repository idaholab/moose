#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import argparse
import numpy as np
import mooseutils
import csv
from scipy.integrate import quad

def command_line_options():
    """
    Command-line options for histogram tool.
    """
    parser = argparse.ArgumentParser(description="Run case study for polynomial chaos with 1D heat conduction model.")
    parser.add_argument('--mpicmd', default='mpiexec', type=str, help="MPI command, default is mpiexec")
    parser.add_argument('--np', default=1, type=int, help="Number of processors to use for calculation, default is 1")
    parser.add_argument('--dist', default='uniform', type=str, choices=['uniform','normal'], help="Type of distribution")
    return parser.parse_args()

def normal_int(x, fun, mu, sig):
    return fun(x)*np.exp(-0.5*((x-mu)/sig)**2)/(sig*np.sqrt(2*np.pi))

if __name__ == '__main__':

    # Command-line options
    opt = command_line_options()

    # Files created
    avg_stats = 'poly_chaos_uniform_out_stats_avg_0001.csv'
    max_stats = 'poly_chaos_uniform_out_stats_max_0001.csv'

    # Commands
    train_mc_cmd = opt.mpicmd + ' -n ' + str(opt.np) + ' ../../stochastic_tools-opt -i poly_chaos_' + opt.dist + '_mc.i Outputs/csv=true'
    train_quad_cmd = opt.mpicmd + ' -n ' + str(opt.np) + ' ../../stochastic_tools-opt -i poly_chaos_' + opt.dist + '_quad.i Outputs/csv=true'
    uniform_cmd = opt.mpicmd + ' -n ' + str(opt.np) + ' ../../stochastic_tools-opt -i poly_chaos_uniform.i Samplers/active=\'\' VectorPostprocessors/active=\'stats_avg stats_max\''

    # Model problem
    if opt.dist in ['uniform']:
        k1 = 1
        k2 = 10
        q1 = 9000
        q2 = 11000
        l1 = 0.01
        l2 = 0.05
        t1 = 290
        t2 = 310
        a = 1.0/3.0
        mu_avg = a * np.log(k2/k1)/(k2-k1) * 0.5*(q2**2-q1**2)/(q2-q1) * 1.0/3.0*(l2**3-l1**3)/(l2-l1) + 0.5*(t2**2-t1**2)/(t2-t1)
        sig_avg = a**2 * (1.0/k1 - 1.0/k2)/(k2-k1) * 1/3.0*(q2**3-q1**3)/(q2-q1) * 1.0/5.0*(l2**5-l1**5)/(l2-l1) \
                        + 2.0*a * np.log(k2/k1)/(k2-k1) * 0.5*(q2**2-q1**2)/(q2-q1) * 1.0/3.0*(l2**3-l1**3)/(l2-l1) * 0.5*(t2**2-t1**2)/(t2-t1) \
                        + 1.0/3.0*(t2**3-t1**3)/(t2-t1) - mu_avg**2
        sig_avg = np.sqrt(sig_avg)
        a = 0.5
        mu_max = a * np.log(k2/k1)/(k2-k1) * 0.5*(q2**2-q1**2)/(q2-q1) * 1.0/3.0*(l2**3-l1**3)/(l2-l1) + 0.5*(t2**2-t1**2)/(t2-t1)
        sig_max = a**2 * (1.0/k1 - 1.0/k2)/(k2-k1) * 1/3.0*(q2**3-q1**3)/(q2-q1) * 1.0/5.0*(l2**5-l1**5)/(l2-l1) \
                        + 2.0*a * np.log(k2/k1)/(k2-k1) * 0.5*(q2**2-q1**2)/(q2-q1) * 1.0/3.0*(l2**3-l1**3)/(l2-l1) * 0.5*(t2**2-t1**2)/(t2-t1) \
                        + 1.0/3.0*(t2**3-t1**3)/(t2-t1) - mu_max**2
        sig_max = np.sqrt(sig_max)
    elif opt.dist in ['normal']:
        km = 5
        ks = 2
        qm = 10000
        qs = 500
        lm = 0.03
        ls = 0.01
        tm = 300
        ts = 10
        qord = 10
        xq, wq = np.polynomial.hermite.hermgauss(qord)
        wq /= np.sum(wq)
        ki = quad(normal_int, -np.inf, np.inf, args=((lambda k: 1.0/k), km, ks))[0]
        k2i = quad(normal_int, -np.inf, np.inf, args=((lambda k: 1.0/k**2), km, ks))[0]
        qi = 0
        q2i = 0
        li = 0
        l2i = 0
        ti = 0
        t2i = 0
        for i in range(len(xq)):
            qq = xq[i]*np.sqrt(2.0)*qs + qm
            qi += qq*wq[i]
            q2i += qq*qq*wq[i]
            lq = xq[i]*np.sqrt(2.0)*ls + lm
            li += lq*lq*wq[i]
            l2i += lq**4*wq[i]
            tq = xq[i]*np.sqrt(2.0)*ts + tm
            ti += tq*wq[i]
            t2i += tq*tq*wq[i]
        mu_avg = 1.0/3.0*ki*qi*li + ti
        sig_avg = np.sqrt(1.0/9.0*k2i*q2i*l2i + t2i + 2.0/3.0*ki*qi*li*ti - mu_avg**2)
        mu_max = 0.5*ki*qi*li + ti
        sig_max = np.sqrt(0.25*k2i*q2i*l2i + t2i + ki*qi*li*ti - mu_max**2)
    print(mu_avg)
    print(sig_avg)
    print(mu_max)
    print(sig_max)

    N = 7
    mc_rows = np.ceil(np.logspace(1, 4, num=N))
    quad_order = np.linspace(3, N+2, num=N)
    quad_type = ['none', 'smolyak']

    data_avg = np.zeros((N, 2, 1+len(quad_type)))
    data_max = np.zeros((N, 2, 1+len(quad_type)))
    points = np.zeros((N, 1+len(quad_type)), dtype=int)
    meth = ['mc', 'tensor', 'smolyak']

    n = -1
    nn = 10
    for nrows in mc_rows:
        n += 1
        points[n,0] = nrows
        mu_avg_err = np.zeros(nn)
        sig_avg_err = np.zeros(nn)
        mu_max_err = np.zeros(nn)
        sig_max_err = np.zeros(nn)
        for i in range(nn):
            cmd = train_mc_cmd + ' Samplers/sample/num_rows=' + str(nrows) + ' Samplers/sample/seed=' + str(i)
            print('MC ' + str(nrows) + ' rows ' + str(i+1) + '/' + str(nn) + ': ')
            os.system(cmd + ' > tmp.txt')
            print('trained ')
            os.system(uniform_cmd + ' > tmp.txt')
            print('finished\n')
            os.system('rm poly_chaos*0000.csv')
            data = mooseutils.PostprocessorReader(avg_stats)
            mu_avg_err[i] = np.absolute((data['value'][0] - mu_avg)/mu_avg)
            sig_avg_err[i] = np.absolute((data['value'][1] - sig_avg)/sig_avg)
            data = mooseutils.PostprocessorReader(max_stats)
            mu_max_err[i] = np.absolute((data['value'][0] - mu_max)/mu_max)
            sig_max_err[i] = np.absolute((data['value'][1] - sig_max)/sig_max)
        data = mooseutils.PostprocessorReader('poly_chaos_training_samp_avg_0001.csv')
        points[n,0] = len(data['sample'])
        data_avg[n,0,0] = np.mean(mu_avg_err)
        data_avg[n,1,0] = np.mean(sig_avg_err)
        data_max[n,0,0] = np.mean(mu_max_err)
        data_max[n,1,0] = np.mean(sig_max_err)

    n = -1
    for ord in quad_order:
        n += 1
        m = 0
        for type in quad_type:
            m += 1
            cmd = train_quad_cmd + ' Samplers/sample/order=' + str(ord) + ' Samplers/sample/sparse_grid=' + type
            print('Quad ' + str(ord) + ' order ' + type + ': ')
            os.system(cmd + ' > tmp.txt')
            print('trained ')
            os.system(uniform_cmd + ' > tmp.txt')
            print('finished\n')
            os.system('rm poly_chaos*0000.csv')
            data = mooseutils.PostprocessorReader('poly_chaos_training_samp_avg_0001.csv')
            points[n,m] = len(data['sample'])
            data = mooseutils.PostprocessorReader(avg_stats)
            data_avg[n,0,m] = np.absolute((data['value'][0] - mu_avg)/mu_avg)
            data_avg[n,1,m] = np.absolute((data['value'][1] - sig_avg)/sig_avg)
            data = mooseutils.PostprocessorReader(max_stats)
            data_max[n,0,m] = np.absolute((data['value'][0] - mu_max)/mu_max)
            data_max[n,1,m] = np.absolute((data['value'][1] - sig_max)/sig_max)

    file_names = ['poly_chaos_avg_' + opt.dist + '_results.csv', 'poly_chaos_max_' + opt.dist + '_results.csv']
    data_full = [data_avg, data_max]
    for i in range(len(file_names)):
        file = file_names[i]
        data = data_full[i]
        with open(file, 'w+') as reader:
            fid = csv.writer(reader)
            head = []
            for me in meth:
                head.append(me + '_points')
                head.append(me + '_mu')
                head.append(me + '_sig')
            fid.writerow(head)
            for n in range(N):
                row = []
                for m in range(len(meth)):
                    row.append(str(points[n,m]))
                    row.append(str(data[n,0,m]))
                    row.append(str(data[n,1,m]))
                fid.writerow(row)

    data_quad = np.zeros((N, 4, len(quad_type)))
    points = np.zeros((N, len(quad_type)), dtype=int)
    n = -1
    for ord in quad_order:
        n += 1
        m = -1
        for type in quad_type:
            m += 1
            cmd = train_quad_cmd + ' Samplers/sample/order=' + str(ord) + ' Samplers/sample/sparse_grid=' + type \
                + ' Trainers/poly_chaos_avg/order=' + str(ord) + ' Trainers/poly_chaos_max/order=' + str(ord)
            print('Quad ' + str(ord) + ' order ' + type + ': ')
            os.system(cmd + ' > tmp.txt')
            print('trained ')
            os.system(uniform_cmd + ' > tmp.txt')
            print('finished\n')
            os.system('rm poly_chaos*0000.csv')
            data = mooseutils.PostprocessorReader('poly_chaos_training_samp_avg_0001.csv')
            points[n,m] = len(data['sample'])
            data = mooseutils.PostprocessorReader(avg_stats)
            data_quad[n,0,m] = np.absolute((data['value'][0] - mu_avg)/mu_avg)
            data_quad[n,1,m] = np.absolute((data['value'][1] - sig_avg)/sig_avg)
            data = mooseutils.PostprocessorReader(max_stats)
            data_quad[n,2,m] = np.absolute((data['value'][0] - mu_max)/mu_max)
            data_quad[n,3,m] = np.absolute((data['value'][1] - sig_max)/sig_max)

    meth = ['tensor','smolyak']
    with open('poly_chaos_order_' + opt.dist + '_results.csv', 'w+') as reader:
        fid = csv.writer(reader)
        head = []
        for me in meth:
            head.append(me + '_points')
            head.append(me + '_mu_avg')
            head.append(me + '_sig_avg')
            head.append(me + '_mu_max')
            head.append(me + '_sig_max')
        fid.writerow(head)
        for n in range(N):
            row = []
            for m in range(len(meth)):
                row.append(str(points[n,m]))
                for i in range(4):
                    row.append(str(data_quad[n,i,m]))
            fid.writerow(row)
