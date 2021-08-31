#!/usr/bin/env python3

try:
    from numpy import linspace
except:
  # np.linspace shim
  def linspace(a,b,n):
    return [a+(b-a)*x/(n-1) for x in range(n)]

def write_list(f, a):
    b = 0
    for v in a:
        if b == 5:
            f.write('\n')
            b = 0
        f.write(str(v) + ' ')
        b += 1

def tabulate_function(filename, func):
    n = 15
    # define an n*n*n*n grid
    axis = [
        ["AXIS X", linspace(0,1,n)],
        ["AXIS Y", linspace(0,1,n)],
        ["AXIS Z", linspace(0,1,n)],
        ["AXIS T", linspace(0,1,n)]
    ]

    f = open(filename + ".data", mode = 'wt')


    # write header
    f.write("# Tabulate " + filename)

    for a in axis:
        f.write('\n\n' + a[0] + '\n')
        f.write(' '.join([str(b) for b in a[1]]))

    f.write('\n\nDATA\n')

    # tabulate data
    eta = [0,0,0,0]
    data = []
    # iterate over entire state space
    for eta[3] in axis[3][1]:
        for eta[2] in axis[2][1]:
            for eta[1] in axis[1][1]:
                for eta[0] in axis[0][1]:
                    data.append(func(eta))

    write_list(f, data)
    f.close()

# parameter (same as in modules/phase_field/test/tests/TotalFreeEnergy/TotalFreeEnergy_2var_test.i)
f0s = 0.125
wGB = 60
GBEnergy = 0.708
length_scale = 1.0e-9
JtoeV = 6.24150974e18
sigma = GBEnergy * JtoeV * length_scale**2
mu = 3.0 / 4.0 * 1.0 / f0s * sigma / wGB;
gamma = 1.5

# compute F
def F(eta):
    sum1 = 0
    for i in range(4):
        sum1 += eta[i]**4/4 - eta[i]/2
    sum2 = 0
    for i in range(4):
        for j in range(i+1, 4):
            sum2 += eta[i]**2 * eta[j]**2
    return mu * (sum1 + gamma * sum2) + 1/4

# compute chemical potential for eta[k]
def dF(eta, k):
    dsum1 =  eta[k]**3 - eta[k]
    dsum2 = 0
    for i in range(4):
        if i != k:
            dsum2 += eta[i]**2
    dsum2 *= 2 * eta[k]
    return mu * (dsum1 + gamma * dsum2)


tabulate_function("grain_growth_energy", F)
tabulate_function("grain_growth_mu0", lambda eta: dF(eta ,0))
tabulate_function("grain_growth_mu1", lambda eta: dF(eta ,1))
tabulate_function("grain_growth_mu2", lambda eta: dF(eta ,2))
tabulate_function("grain_growth_mu3", lambda eta: dF(eta ,3))
