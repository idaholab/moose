from sympy import * # use star so all functions are available to supplied strings
from sympy.vector import divergence, gradient, Vector, CoordSys3D
from fparser import print_fparser
from moosefunction import print_moose

def evaluate(pde, soln, variable='u', scalars=set(), vectors=set(), functions=set(), vectorfunctions=set()):
    """
    Function to evaluate PDEs for the method of manufactured solutions forcing functions.

    Inputs:
        pde[str]: A string representation of your PDE, eg. 'diff(T,t) + div(grad(T))'
        soln[str]: The desired solution, e.g., 'cos(u*t*x)'
        variable[str]: The solution variable in the PDE (default: 'u')
        scalars[list]: A list of strings of constant extra scalar variables
        vectors[list]: A list of strings of extra constant vector variables
        functions[list]: A list of strings of scalar variables that are a function of x,y,z, and t.
        vector_functions[list]: A list of strings of generic vector variables that are a function of
                                x,y,z, and t.

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

    # Define extra vectors, use _v_ in order to not collide with vector=['v']
    for _v_ in vectors:
        for _c_ in 'xyz':
            _s_ = '{}_{}'.format(_v_, _c_)
            locals()[_s_] = Symbol(_s_)
        locals()[_v_] = locals()['{}_x'.format(_v_)]*R.i + \
                        locals()['{}_y'.format(_v_)]*R.j + \
                        locals()['{}_z'.format(_v_)]*R.k

    # Define extra scalars
    for _s_ in scalars:
        locals()[_s_] = Symbol(_s_)

    # Define extra functions
    for _f_ in functions:
        locals()[_f_] = Function(_f_)(x, y, z, t)

    for _vf_ in vectorfunctions:
        for _c_ in 'xyz':
            _s_ = '{}_{}'.format(_vf_, _c_)
            locals()[_s_] = Function(_s_)(x, y, z, t)
        locals()[_vf_] = locals()['{}_x'.format(_vf_)]*R.i + \
                         locals()['{}_y'.format(_vf_)]*R.j + \
                         locals()['{}_z'.format(_vf_)]*R.k

    # Evaluate the solution
    locals()[variable] = eval(soln)

    # Evaluate the PDE
    pde = pde.replace('grad', 'gradient')
    pde = pde.replace('div', 'divergence')
    return eval(pde), locals()[variable]
