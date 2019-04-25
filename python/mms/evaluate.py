from sympy import * # use star so all functions are available to supplied strings
from sympy.vector import divergence, gradient, Vector, CoordSys3D
from fparser import print_fparser
from moosefunction import print_moose

def evaluate(pde, soln, variable='u',
             scalars=set(),
             vectors=set(),
             functions=set(),
             vectorfunctions=set(),
             negative=False,
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

    # Define the standard symbols
    R = CoordSys3D('R')
    x = R.x
    y = R.y
    z = R.z
    t = Symbol('t')
    e_i = R.i
    e_j = R.j
    e_k = R.k

    # Define extra vectors, use _v_ in order to not collide with vector=['v']
    for _v_ in vectors:
        _check_reserved(_v_)
        for _c_ in 'xyz':
            _s_ = '{}_{}'.format(_v_, _c_)
            locals()[_s_] = Symbol(_s_)
        locals()[_v_] = locals()['{}_x'.format(_v_)]*R.i + \
                        locals()['{}_y'.format(_v_)]*R.j + \
                        locals()['{}_z'.format(_v_)]*R.k

    # Define extra scalars
    for _s_ in scalars:
        _check_reserved(_s_)
        locals()[_s_] = Symbol(_s_)

    # Define extra functions
    for _f_ in functions:
        _check_reserved(_f_)
        locals()[_f_] = Function(_f_)(x, y, z, t)

    # Define extra vector functions
    for _vf_ in vectorfunctions:
        _check_reserved(_vf_)
        for _c_ in 'xyz':
            _s_ = '{}_{}'.format(_vf_, _c_)
            locals()[_s_] = Function(_s_)(x, y, z, t)
        locals()[_vf_] = locals()['{}_x'.format(_vf_)]*R.i + \
                         locals()['{}_y'.format(_vf_)]*R.j + \
                         locals()['{}_z'.format(_vf_)]*R.k

    # Define known functions
    for _f_, _v_ in kwargs.iteritems():
        _check_reserved(_f_)
        locals()[_f_] = eval(_v_)
        if isinstance(locals()[_f_], Vector):
            locals()['{}_x'.format(_f_)] = locals()[_f_].components.get(R.i, 0)
            locals()['{}_y'.format(_f_)] = locals()[_f_].components.get(R.j, 0)
            locals()['{}_z'.format(_f_)] = locals()[_f_].components.get(R.k, 0)

    locals()[variable] = eval(soln)

    # Evaluate the PDE
    pde = pde.replace('grad', 'gradient')
    pde = pde.replace('div', 'divergence')
    if negative:
        return -eval(pde), locals()[variable]
    else:
        return eval(pde), locals()[variable]

def _check_reserved(var):
    """Error checking for input variables."""
    if var == 'R':
        raise SyntaxError("The variable name 'R' is reserved, it represents the coordinate system," \
                          " see sympy.vector.CoordSys3D.")

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
