# LatinHypercube

!syntax description /Samplers/LatinHypercube

## Overview

This sampler implements the Latin hypercube strategy presented in [!cite](mckay1979comparison) as:

> If we wish to ensure also that each of the input variables $X_k$ has all portions of its distribution
> represented by input values, we can divide the range of each $X_k$ into $N$ strata of equal marginal
> probability $l/N$, and sample once from each stratum. Let this sample be
> $X_{kj},\,j=l,\ldots,N$. These form the $X_k$, component, $k=l,\ldots,K$, in $X_i,\,i =
> 1,\ldots,N$. The components of the various $X_k$'s are matched at random.

## Example Input File Syntax

The following input file creates a Latin hypercube sample for two distributions, where the
supplied uniform distribution is sampled with six bins with probabilities between 0 and 1. The
normal distribution is sampled with seven bins with probabilities between 0.001 and 0.999.

!listing latin_hypercube.i block=Distributions Samplers

The graph in [hypercube] show the Latin hypercube sample generated from the input file snippet where
the grid lines are defined with the associated probability based bins.

!plot scatter filename=gold/latin_hypercube_out_data_0001.csv id=hypercube
      caption=Latin hypercube sampled data with tick marks defined at the probability intervals.
      data=[{'x':'sample_0', 'y':'sample_1', 'mode':'markers'}]
      layout={'xaxis':{'title':'Uniform', 'linewidth':1, 'showline':1, 'tickmode':'array', 'tickvals':[2004,2005,2006,2007,2008,2009,2010]},
              'yaxis':{'title':'Normal', 'linewidth':1, 'showline':1, 'tickmode':'array', 'tickvals':[1970.7,1976.8,1978.3,1979.5,1980.5,1981.7,1983.2,1989.3]},
              'width':775, 'height':800, 'font':{'size':16}, 'margin':{'l':100}}


!syntax parameters /Samplers/LatinHypercube

!syntax inputs /Samplers/LatinHypercube

!syntax children /Samplers/LatinHypercube
