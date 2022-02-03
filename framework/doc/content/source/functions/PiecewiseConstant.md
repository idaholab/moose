# PiecewiseConstant

!syntax description /Functions/PiecewiseConstant

## Description

The `PiecewiseConstant` function defines the data using a set of x-y data pairs.  Instead
of linearly interpolating between the values, however, the `PiecewiseConstant` function
is constant when the abscissa is between the values provided by the user.  The `direction`
parameter controls whether the function takes the value of the abscissa of the
user-provided point to the `right` or `left` value at which the function is evaluated.
Also available is `right_inclusive` and `left_inclusive` options, which will return the value
of the function at the specified abscissa.

!plot scatter data=[{'x':[-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10], 'y':[8,8,8,6,6,4,4,4,2,2,2,2,2,2,2,2,4,4,6,8,8], 'name':'left', 'marker':{'size':10}, 'mode': 'lines+markers'},
                    {'x':[-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10], 'y':[8,8,8,8,6,4,4,4,4,2,2,2,2,2,2,2,4,6,8,8,8], 'name':'left_inclusive', 'marker':{'size':8}, 'mode': 'lines+markers'},
                    {'x':[-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10], 'y':[8,8,8,6,4,2,2,2,2,2,2,2,4,4,4,4,6,8,8,8,8], 'name':'right', 'marker':{'size':6}, 'mode': 'lines+markers'},
                    {'x':[-10,-9,-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7,8,9,10], 'y':[8,8,6,4,4,2,2,2,2,2,2,2,2,4,4,4,6,6,8,8,8], 'name':'right_inclusive', 'marker':{'size':4}, 'mode': 'lines+markers'},
                    {'x':[-8,-7,-5.5,-2,2,5.5,7,8], 'y':[8,6,4,2,2,4,6,8], 'name':'input', 'marker':{'size':12}, 'mode': 'markers'}]
                    layout={'xaxis':{'title':'Time'},
                            'yaxis':{'title':'Function Value'},
                            'title':'PiecewiseConstant Function options'}




## Example Input Syntax

!listing test/tests/functions/piecewise_constant/piecewise_constant.i block=Functions

!syntax parameters /Functions/PiecewiseConstant

!syntax inputs /Functions/PiecewiseConstant

!syntax children /Functions/PiecewiseConstant
