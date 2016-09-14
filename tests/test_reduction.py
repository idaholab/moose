# This tests is used to verify the svd decomposition calculated inside crow
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
cov = [1.36,   -0.16,  0.21,  0.43,    -0.144,
       -0.16, 6.59,  0.794,  -0.173,  -0.406,
       0.21,  0.794,   5.41, 0.461,   0.179,
       0.43,   -0.173,  0.461,  14.3,   0.822,
       -0.144, -0.406,  0.179,  0.822,   3.75]

size = 5
rank = 3

# Transform 'mu' and 'cov' to the c++ vector
muCpp = distribution1D.vectord_cxx(len(mu))
for i in range(len(mu)):
  muCpp[i] = mu[i]
covCpp = distribution1D.vectord_cxx(len(cov))
for i in range(len(cov)):
  covCpp[i] = cov[i]

# call the functions from the crow to compute the svd
covType = "abs"
mvnDistribution = distribution1D.BasicMultivariateNormal(covCpp,muCpp,str(covType),rank)

dim = mvnDistribution.getSingularValuesDimension()
sCpp_vector = mvnDistribution.getSingularValues()
sCpp = [sCpp_vector[i] for i in range(dim)]
sCpp = np.asarray(sCpp)

dimVectorLeft = mvnDistribution.getLeftSingularVectorsDimensions()
uCpp_vector = mvnDistribution.getLeftSingularVectors()
uCpp = [uCpp_vector[i] for i in range(dimVectorLeft[0]*dimVectorLeft[1])]
uCpp = np.asarray(uCpp)
uCpp = np.reshape(uCpp,(dimVectorLeft[0],dimVectorLeft[1]))

dimVectorRight = mvnDistribution.getRightSingularVectorsDimensions()
vCpp_vector = mvnDistribution.getRightSingularVectors()
vCpp = [vCpp_vector[i] for i in range(dimVectorRight[0]*dimVectorRight[1])]
vCpp = np.asarray(vCpp)
vCpp = np.reshape(vCpp,(dimVectorRight[0],dimVectorRight[1]))

# using numpy to compute the svd
covNp = np.asarray(cov).reshape(-1,sqrt(len(cov)))
uNp,sNp,vNp = LA.svd(covNp,full_matrices=False)

# reconstruct the matrix using A = U*S*V.T using the results from crow
covReCompute = np.dot(uCpp,np.dot(np.diag(sCpp),vCpp.T))

# reconstruct the matrix using A = U*S*V.T using the results from numpy
covNpReCompute = np.dot(uNp[:,0:rank],np.dot(np.diag(sNp[0:rank]),vNp[0:rank,:]))



results = {"pass":0,"fail":0}

utils.checkArrayAllClose("MVN singular values",sCpp,sNp[0:rank],results)
utils.checkArrayAllClose("MVN left singular vectors",np.absolute(uCpp),np.absolute(uNp[:,0:rank]),results)
utils.checkArrayAllClose("MVN right singular vectors", np.absolute(vCpp),np.absolute(vNp[0:rank,:].T),results)
utils.checkArrayAllClose("MVN singular value decomposition",covNpReCompute,covReCompute,results)

utils.checkAnswer("MVN dimensions of singular values",dim,rank,results)
utils.checkAnswer("MVN row dimensions of left singular vectors",dimVectorLeft[0],size,results)
utils.checkAnswer("MVN col dimensions of left singular vectors",dimVectorLeft[1],rank,results)
utils.checkAnswer("MVN row dimensions of right singular vectors",dimVectorRight[0],size,results)
utils.checkAnswer("MVN col dimensions of right singular vectors",dimVectorRight[1],rank,results)

print(results)

sys.exit(results["fail"])
