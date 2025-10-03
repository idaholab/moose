#!/usr/bin/env python3
import time
import numpy as np
np.set_printoptions(precision=16, linewidth=np.inf)

def gfunc(q, x):
    y = 1
    for i in range(len(x)):
        y *= (np.absolute(4*x[i] - 2) + q[i]) / (1 + q[i])
    return y

def evaluate(q, M):
    a = np.ndarray(shape=M.shape[0])
    a[:] = [gfunc(q, M[i,:]) for i in range(M.shape[0])]
    return a

def compute(q, M1, M2):
    n = len(q)
    K = M1.shape[0]

    a_0 = evaluate(q, M2)
    a_K = evaluate(q, M1)
    a_i = np.ndarray(shape=(K, n))
    a_ni = np.ndarray(shape=(K, n))

    for i in range(n):
        Ni = np.copy(M2)
        Ni[:,i] = np.copy(M1[:,i])
        a_i[:,i] = evaluate(q, Ni)

        Nni = np.copy(M1)
        Nni[:,i] = np.copy(M2[:,i])
        a_ni[:,i] = evaluate(q, Nni)

    # First order
    E2 = 1/K * np.dot(a_0, a_K)
    V = 1/K * np.dot(a_K, a_K) - E2
    S = np.ndarray(shape=(n,n))

    for i in range(n):
        U1 = 1/K * np.dot(a_K, a_i[:,i])
        U2 = 1/K * np.dot(a_0, a_ni[:,i])
        S[i,i] = (np.mean([U1, U2]) - E2) / V

    # Second order
    for i in range(n):
        E2 = 1/K * np.dot(a_i[:,i], a_ni[:,i])
        for j in range(i):
            V = 1/K * np.dot(a_ni[:,j], a_ni[:,j]) - E2
            U1 = 1/K * np.dot(a_i[:,i], a_ni[:,j])
            U2 = 1/K * np.dot(a_i[:,j], a_ni[:,i])
            Sc = (np.mean([U1, U2]) - E2) / V
            S[i,j] = Sc - S[i,i] - S[j,j]

    # Total effect
    E2 = 1/K * np.dot(a_0, a_K)
    V = 1/K * np.dot(a_0, a_0) - E2
    ST = [0]*n
    for i in range(n):
        U1 = 1/K * np.dot(a_0, a_i[:,i])
        U2 = 1/K * np.dot(a_K, a_ni[:,i])
        ST[i] = 1 - (np.mean([U1, U2]) - E2) / V

    return S, ST;


if __name__ == '__main__':
    q = [0, 0.5, 3, 9, 99, 99]
    n = len(q)  # number of variables
    K = 1000  # number of replicates

    # Build sample matrices
    np.random.seed(1980)
    M1 = np.random.uniform(0, 1, size=(K, n))
    np.random.seed(1949)
    M2 = np.random.uniform(0, 1, size=(K, n))

    # Compute indices
    S, ST = compute(q, M1, M2)

    # Build gtest statements
    idx = 0;
    for i in range(n):
        print('  EXPECT_NEAR(sobol[{:d}], {:.9f}, 1e-9); // S_{}'.format(idx, S[i,i], i+1))
        idx += 1

    for i in range(n):
        print('  EXPECT_NEAR(sobol[{:d}], {:.9f}, 1e-9); // ST_{}'.format(idx, ST[i], i+1))
        idx += 1

    for i in range(n):
        for j in range(i):
            print('  EXPECT_NEAR(sobol[{:d}], {:.9f}, 1e-9); // S_{}{};'.format(idx, S[i,j], i+1, j+1))
            idx += 1

    # Do bootstrapping
    levels = [0.05, 0.95]
    replicates = 10000
    S_ci = np.ndarray(shape=(len(levels),n,n))
    ST_ci = np.ndarray(shape=(len(levels),n))

    S_rep = np.ndarray(shape=(replicates,n,n))
    ST_rep = np.ndarray(shape=(replicates,n))
    np.random.seed(1993)
    for r in range(replicates):
        ind = np.random.randint(K, size=K)
        M1_rep = np.copy(M1)[ind,:]
        M2_rep = np.copy(M2)[ind,:]
        S_rep[r,:,:], ST_rep[r,:] = compute(q, M1_rep, M2_rep)

    for i in range(n):
        for j in range(n):
            S_rep[:,i,j] = np.sort(S_rep[:,i,j])
        ST_rep[:,i] = np.sort(ST_rep[:,i])

    lnd = np.rint(np.array(levels) * (replicates - 1)).astype(int)
    S_ci = S_rep[lnd,:,:]
    ST_ci = ST_rep[lnd,:]

    # Build gtest statements
    idx = 0;
    for i in range(n):
        print('    // S_{}'.format(i+1))
        print('    EXPECT_NEAR(sobol[{:d}], {:.9f}, 1e-9); // Mean'.format(idx, S[i,i]))
        for l in range(len(levels)):
            print('    EXPECT_NEAR(sobol_ci[{:d}][{:d}], {:.9f}, 1e-9); // {:.0f}% CI'.format(l, idx, S_ci[l,i,i], levels[l] * 100))
        idx += 1

    print('')
    for i in range(n):
        print('    // ST_{}'.format(i+1))
        print('    EXPECT_NEAR(sobol[{:d}], {:.9f}, 1e-9); // Mean'.format(idx, ST[i]))
        for l in range(len(levels)):
            print('    EXPECT_NEAR(sobol_ci[{:d}][{:d}], {:.9f}, 1e-9); // {:.0f}% CI'.format(l, idx, ST_ci[l,i], levels[l] * 100))
        idx += 1

    print('')
    for i in range(n):
        for j in range(i):
            print('    // S_{}{}'.format(i+1, j+1))
            print('    EXPECT_NEAR(sobol[{:d}], {:.9f}, 1e-9); // Mean'.format(idx, S[i,j]))
            for l in range(len(levels)):
                print('    EXPECT_NEAR(sobol_ci[{:d}][{:d}], {:.9f}, 1e-9); // {:.0f}% CI'.format(l, idx, S_ci[l,i,j], levels[l] * 100))
            idx += 1
