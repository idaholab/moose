from sympy import *
from sympy.vector import divergence, gradient, Vector, CoordSys3D
from fparser import print_fparser
from moosefunction import print_moose

def evaluate(pde, soln, variable='u', scalars=set(), vectors=set()):
    """
    Function to evaluate PDEs for the method of manufactured solutions forcing functions.

    Inputs:
        pde[str]: A string representation of your PDE, eg. 'diff(T,t) + div(grad(T))'
        soln[str]: The desired solution, e.g., 'cos(u*t*x)'
        variable[str]: The solution variable in the PDE (default: 'u')
        scalars[list]: A list of strings of extra scalar variables, the symbols 'x', 'y', 'z', and
                       't' are automatically defined
        vectors

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

    for v in vectors:
        for c in 'xyz':
            s = '{}_{}'.format(v,c)
            locals()[s] = Symbol(s)
        locals()[v] = locals()['{}_x'.format(v)]*R.i + \
                      locals()['{}_y'.format(v)]*R.j + \
                      locals()['{}_z'.format(v)]*R.k

    # Define extra symbols
    for s in scalars:
        locals()[s] = Symbol(s)

    # Evaluate the solution
    locals()[variable] = eval(soln)

    # Evaluate the PDE
    pde = pde.replace('grad', 'gradient')
    pde = pde.replace('div', 'divergence')
    return eval(pde)


if __name__ == '__main__':
    f = evaluate('diff(h, t) + div(u*h)', 'cos(x*y*t)', variable='h', vectors='u')
    #print_moose(f)
    print_fparser(f)
