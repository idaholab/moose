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

The following input file creates a Latin hypercube sample from two uniform distributions with
10 samples of each distribution.

!listing latin_hypercube.i block=Distributions Samplers

The graph in [hypercube] show the Latin hypercube sample generated from the input file snippet where
the grid lines are defined with the associated probability based bins.

!plot scatter filename=gold/latin_hypercube_out_data_0001.csv id=hypercube
      caption=Latin hypercube sampled data with tick marks defined at the probability intervals.
      data=[{'x':'sample_0', 'y':'sample_1', 'mode':'markers'}]
      layout={'xaxis':{'title':'Uniform "a" (100,200)', 'linewidth':1, 'showline':1, 'tickmode':'array', 'tickvals':[100,110,120,130,140,150,160,170,180,190,200]},
              'yaxis':{'title':'Uniform "b" (10,20)', 'linewidth':1, 'showline':1, 'tickmode':'array', 'tickvals':[10,11,12,13,14,15,16,17,18,19,20]},
              'width':775, 'height':800, 'font':{'size':16}, 'margin':{'l':100}}


!syntax parameters /Samplers/LatinHypercube

!syntax inputs /Samplers/LatinHypercube

!syntax children /Samplers/LatinHypercube
