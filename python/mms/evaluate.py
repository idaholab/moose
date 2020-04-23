#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from sympy import * # use star so all functions are available to supplied strings
from sympy.vector import divergence, gradient, Vector, CoordSys3D
from mms.fparser import print_fparser
from mms.moosefunction import print_moose

def evaluate(pde, soln, variable='u',
             scalars=set(),
             vectors=set(),
             functions=set(),
             vectorfunctions=set(),
             negative=False,
             transformation='cartesian',
             coordinate_names=("x", "y", "z"),
             **kwargs):
    """
    Function to evaluate PDEs for the method of manufactured solutions forcing functions.

    Inputs:
        pde[str]: A string representation of your PDE, eg. 'diff(T,t) + div(grad(T))'
        soln[str]: The desired solution, e.g., 'cos(u*t*x)'
        variable[str]: The solution variable in the PDE (default: 'u')
        scalars[list]: A list of strings of constant scalar variables
        vectors[list]: A list of strings of extra constant vector variables
        functions[list]: A list of strings of arbitrary scalar variables that are a function of
                         x,y,z, and t.
        vectorfunctions[list]: A list of strings of arbitrary vector variables that are a function
                               of x,y,z, and t.
        negative[bool]: If true the negative of the computed function is returned, by default this
                        is false. Thus, by default the return function may be pasted directly
                        into the BodyForce Kernel object within MOOSE.
        kwargs: Variables with known functions that are a function of x,y,z, and t or any other
                symbol, e.g. "u='cos(x*y)'" or "y='2*y*y*R.i + 5*x*R.j'".

    Example:
        import mms
        f = mms.evaluate('diff(T, t) + div(grad(T))', 'cos(u*t*x)', variable='T', scalars=['u'])
        mms.print_moose(f)
        mms.print_fparser(f)
    """

    if len(coordinate_names) != 3:
        raise SyntaxError('The coordinate_names argument must be of length 3')

    if transformation != 'cartesian' and transformation != 'cylindrical':
        raise SyntaxError('transformation must be either cartesian or cylindrical')

    # Define the standard symbols
    R = CoordSys3D('R', transformation=transformation, variable_names=coordinate_names)

    # string names of variables
    sx1 = coordinate_names[0]
    sx2 = coordinate_names[1]
    sx3 = coordinate_names[2]

    # symbols with short names for use in code below
    x1 = getattr(R, coordinate_names[0])
    x2 = getattr(R, coordinate_names[1])
    x3  = getattr(R, coordinate_names[2])

    # necessary declaration of names needed when running `eval`
    locals()[coordinate_names[0]] = x1
    locals()[coordinate_names[1]] = x2
    locals()[coordinate_names[2]] = x3

    t = Symbol('t')
    e_i = R.i
    e_j = R.j
    e_k = R.k

    # Define extra vectors, use _v_ in order to not collide with vector=['v']
    for _v_ in vectors:
        _check_reserved(_v_)
        for _c_ in [sx1, sx2, sx3]:
            _s_ = '{}_{}'.format(_v_, _c_)
            locals()[_s_] = Symbol(_s_)
        locals()[_v_] = locals()['{}_{}'.format(_v_,sx1)]*R.i + \
                        locals()['{}_{}'.format(_v_,sx2)]*R.j + \
                        locals()['{}_{}'.format(_v_,sx3)]*R.k

    # Define extra scalars
    for _s_ in scalars:
        _check_reserved(_s_)
        locals()[_s_] = Symbol(_s_)

    # Define extra functions
    for _f_ in functions:
        _check_reserved(_f_)
        locals()[_f_] = Function(_f_)(x1, x2, x3, t)

    # Define extra vector functions
    for _vf_ in vectorfunctions:
        _check_reserved(_vf_)
        for _c_ in [sx1, sx2, sx3]:
            _s_ = '{}_{}'.format(_vf_, _c_)
            locals()[_s_] = Function(_s_)(x1, x2, x3, t)
        locals()[_vf_] = locals()['{}_{}'.format(_vf_,sx1)]*R.i + \
                         locals()['{}_{}'.format(_vf_,sx2)]*R.j + \
                         locals()['{}_{}'.format(_vf_,sx3)]*R.k

    # Define known functions
    for _f_, _v_ in kwargs.items():
        _check_reserved(_f_)
        locals()[_f_] = eval(_v_)
        if isinstance(locals()[_f_], Vector):
            locals()['{}_{}'.format(_f_,sx1)] = locals()[_f_].components.get(R.i, 0)
            locals()['{}_{}'.format(_f_,sx2)] = locals()[_f_].components.get(R.j, 0)
            locals()['{}_{}'.format(_f_,sx3)] = locals()[_f_].components.get(R.k, 0)

    # Evaluate the supplied solution
    _exact_ = eval(soln)
    locals()[variable] = _exact_

    # Evaluate the PDE
    pde = pde.replace('grad', 'gradient')
    pde = pde.replace('div', 'divergence')
    _func_ = eval(pde)
    if negative:
        _func_ = -1 * _func_

    # Convert vector exact solution to a list
    if isinstance(_exact_, Vector):
        _exact_ = [_exact_.components.get(R.i, 0),
                   _exact_.components.get(R.j, 0),
                   _exact_.components.get(R.k, 0)]

    # Convert vector result to a list
    if isinstance(_func_, Vector):
        _func_ = [_func_.components.get(R.i, 0),
                  _func_.components.get(R.j, 0),
                  _func_.components.get(R.k, 0)]

    return _func_, _exact_

def _check_reserved(var):
    """Error checking for input variables."""
    if var == 'R':
        msg = "The variable name 'R' is reserved, it represents the coordinate system, " \
              "see sympy.vector.CoordSys3D."
        raise SyntaxError(msg)

    elif var in ['x', 'y', 'z']:
        msg = "The variable name '{0}' is reserved, it represents the {0} spatial direction " \
              "(R.{0}) for the 'R' coordinate system as defined by a sympy.vector.CoordSys3D."
        raise SyntaxError(msg.format(var))

    elif var == 't':
        raise SyntaxError("The variable name 't' is reserved, it represents time.")


    elif var in ['e_i', 'e_j', 'e_k']:
        basis = dict(e_i='x', e_j='y', e_k='z')
        msg = "The variable name '{0}' is reserved, it represents the {1} basis vector " \
              "(R.{0}) for the 'R' coordinate system as defined by a sympy.vector.CoordSys3D."
        raise SyntaxError(msg.format(var, basis[var]))
