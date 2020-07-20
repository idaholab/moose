# POD Reduced Basis Surrogate

This example is meant to demonstrate how a POD reduced basis surrogate model is trained
and used on a parametric problem.

## Problem statement

The full-order model is a one energy group, fixed-source neutron diffusion problem, adopted from [!cite](prince2019parametric).
It showcases a Pressurized Water Reactor (PWR) reactor core surrounded with water reflector.

!media stochastic_tools/surrogates/pod_rb/geometry.png style=width:100%; id=fig:geometry
       caption=Something here.



## Solving the problem without uncertain parameters

## Training surrogate models

## Evaluation of surrogate models

## Results and Analysis



!plot scatter
  filename=examples/surrogates/pod_rb/2d_multireg/gold/2d_multireg_results.csv
  data=[{'x':'no_modes', 'y':'mean', 'name':'Average relative error'},
        {'x':'no_modes', 'y':'max', 'name':'Maximum relative error'}]
  layout={'xaxis':{'type':'log', 'title':'Number of basis functions'},
          'yaxis':{'type':'log','title':'Relative Error'}}
