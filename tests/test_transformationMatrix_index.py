# This test is used to verify the svd decomposition calculated inside crow
# the svd module from numpy.linalg is used as gold solution.

#For future compatibility with Python 3
from __future__ import division, print_function, unicode_literals, absolute_import
import warnings
warnings.simplefilter('default',DeprecationWarning)
#!/usr/bin/env python

import sys
import utils
import numpy as np
from math import sqrt
from numpy import linalg as LA

distribution1D = utils.find_distribution1D()
# input data, random matrix can also be used.
mu = [1.0,2.0,3.0,4.0,5.0]
cov = [1.36,   -0.816,  0.521,  1.43,    -0.144,
       -0.816, -0.659,  0.794,  -0.173,  -0.406,
       0.521,  0.794,   -0.541, 0.461,   0.179,
       1.43,   -0.173,  0.461,  -1.43,   0.822,
       -0.144, -0.406,  0.179,  0.822,   -1.37]

index = [3,4,1,2]

# Transform 'mu' and 'cov' to the c++ vector
muCpp = distribution1D.vectord_cxx(len(mu))
for i in range(len(mu)):
  muCpp[i] = mu[i]
covCpp = distribution1D.vectord_cxx(len(cov))
for i in range(len(cov)):
  covCpp[i] = cov[i]
indexCpp = distribution1D.vectori_cxx(len(index))
for i in range(len(index)):
  indexCpp[i] = index[i]

# call the functions from the crow to compute the svd
covType = "abs"
rank = 5
mvnDistribution = distribution1D.BasicMultivariateNormal(covCpp,muCpp,str(covType),5)

dimVector = mvnDistribution.getTransformationMatrixDimensions(indexCpp)
uCpp_vector = mvnDistribution.getTransformationMatrix(indexCpp)
uCpp = [uCpp_vector[i] for i in range(dimVector[0]*dimVector[1])]
uCpp = np.asarray(uCpp)
uCpp = np.reshape(uCpp,(dimVector[0],dimVector[1]))

# using numpy to compute the svd
covNp = np.asarray(cov).reshape(-1,sqrt(len(cov)))
uNp,sNp,vNp = LA.svd(covNp,full_matrices=False)

# compute the transformation matrix  = U*sqrt(S)
uReCompute = np.dot(uNp[:,index],np.sqrt(np.diag(sNp[index])))

results = {"pass":0,"fail":0}

utils.checkArrayAllClose("MVN transformation matrix",np.absolute(uCpp),np.absolute(uReCompute),results)
utils.checkAnswer("MVN row dimensions of transformation matrix",dimVector[0],5,results)
utils.checkAnswer("MVN col dimensions of transformation matrix",dimVector[1],len(index),results)

print(results)

sys.exit(results["fail"])
