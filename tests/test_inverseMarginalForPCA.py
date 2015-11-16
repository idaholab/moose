#For future compatibility with Python 3
from __future__ import division, print_function, unicode_literals, absolute_import
import warnings
warnings.simplefilter('default',DeprecationWarning)

import sys
import utils

distribution1D = utils.find_distribution1D()
# input data, random matrix can also be used.
mu = [1.0,2.0,3.0,4.0,5.0]
cov = [1.36,   -0.816,  0.521,  1.43,    -0.144,
       -0.816, -0.659,  0.794,  -0.173,  -0.406,
       0.521,  0.794,   -0.541, 0.461,   0.179,
       1.43,   -0.173,  0.461,  -1.43,   0.822,
       -0.144, -0.406,  0.179,  0.822,   -1.37]

# Transform 'mu' and 'cov' to the c++ vector
muCpp = distribution1D.vectord_cxx(len(mu))
for i in range(len(mu)):
  muCpp[i] = mu[i]
covCpp = distribution1D.vectord_cxx(len(cov))
for i in range(len(cov)):
  covCpp[i] = cov[i]

# call the functions from the crow to compute the svd
covType = "abs"
rank = 4
mvnDistribution = distribution1D.BasicMultivariateNormal(covCpp,muCpp,str(covType),rank)

results = {"pass":0,"fail":0}

utils.checkAnswer("multivariate norm inverse marginal for pca (0.1)",mvnDistribution.inverseMarginalForPCA(0.1),-1.2815515655446004,results)
utils.checkAnswer("multivariate norm inverse marginal for pca (0.25)",mvnDistribution.inverseMarginalForPCA(0.25),-0.67448975019608171,results)
utils.checkAnswer("multivariate norm inverse marginal for pca (0.5)",mvnDistribution.inverseMarginalForPCA(0.5),0.0,results)
utils.checkAnswer("multivariate norm inverse marginal for pca (0.75)",mvnDistribution.inverseMarginalForPCA(0.75),0.67448975019608171,results)
utils.checkAnswer("multivariate norm inverse marginal for pca (0.9)",mvnDistribution.inverseMarginalForPCA(0.9),1.2815515655446004,results)

print(results)

sys.exit(results["fail"])
