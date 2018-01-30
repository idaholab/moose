#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import sympy as sp
import re

'''
Calculus methods
'''

def eye2():
    return sp.Matrix([[sp.Integer(1), sp.Integer(0)], [sp.Integer(0), sp.Integer(1)]])

def zeroVec2():
    return sp.Matrix([sp.Integer(0), sp.Integer(0)])

def gradVec2(u_vec, x, y):
    return sp.Matrix([[sp.diff(u_vec[0], x), sp.diff(u_vec[1],x)], [sp.diff(u_vec[0], y), sp.diff(u_vec[1], y)]])

def divTen2(tensor, x, y):
    return sp.Matrix([sp.diff(tensor[0,0], x) + sp.diff(tensor[1,0], y), sp.diff(tensor[0, 1], x) + sp.diff(tensor[1,1], y)])

def divVec2(u_vec, x, y):
    return sp.diff(u_vec[0], x) + sp.diff(u_vec[1], y)

def gradScalar2(u, x, y):
    return sp.Matrix([sp.diff(u, x), sp.diff(u,y)])

def strain_rate(u_vec, x, y):
    return gradVec2(u_vec, x, y) + gradVec2(u_vec, x, y).transpose()

def strain_rate_squared_2(u_vec, x, y):
    tensor = gradVec2(u_vec, x, y) + gradVec2(u_vec, x, y).transpose()
    rv = 0
    for i in range(2):
        for j in range(2):
            rv += tensor[i, j] * tensor[i, j]
    return rv

def laplace2(u, x, y):
    return sp.diff(sp.diff(u, x), x) + sp.diff(sp.diff(u, y), y)

'''
Kernel operators and corresponding surface integral terms
'''

def L_advection(u, x, y):
    ax, ay = sp.var('ax ay')
    return sp.Matrix([ax, ay]).transpose() * sp.Matrix([sp.diff(u, x), sp.diff(u, y)])

def L_diffusion(u, x, y):
    return -laplace2(u, x, y)

def bc_terms_diffusion(u, nvec, x, y):
    return (-nvec.transpose() * gradScalar2(u, x, y))[0,0]

def L_momentum_traction(uvec, p, k, eps, x, y):
    cmu = 0.09
    mu, rho = sp.var('mu rho')
    visc_term = (-mu * divTen2(gradVec2(uvec, x, y) + gradVec2(uvec, x, y).transpose(), x, y)).transpose()
    conv_term = rho * uvec.transpose() * gradVec2(uvec, x, y)
    pressure_term = gradScalar2(p, x, y).transpose()
    turbulent_visc_term = -(divTen2(rho * cmu * k**2 / eps * (gradVec2(uvec, x, y) + gradVec2(uvec, x, y).transpose()), x, y)).transpose()
    source = conv_term + visc_term + pressure_term + turbulent_visc_term
    return source

def bc_terms_momentum_traction(uvec, nvec, p, k, eps, x, y, symbolic=True, parts=True):
    if symbolic:
        cmu = sp.var('c_{\mu}')
    else:
        cmu = 0.09
    mu, rho = sp.var('mu rho')
    visc_term = (-mu * nvec.transpose() * (gradVec2(uvec, x, y) + gradVec2(uvec, x, y).transpose())).transpose()
    if parts:
        pressure_term = (nvec.transpose() * eye2() * p).transpose()
    else:
        pressure_term = zeroVec2()
    turbulent_visc_term = -(nvec.transpose() * (rho * cmu * k**2 / eps * (gradVec2(uvec, x, y) + gradVec2(uvec, x, y).transpose()))).transpose()
    return visc_term + turbulent_visc_term + pressure_term

def L_momentum_traction_no_turbulence(uvec, p, x, y):
    mu, rho = sp.var('mu rho')
    visc_term = (-mu * divTen2(gradVec2(uvec, x, y) + gradVec2(uvec, x, y).transpose(), x, y)).transpose()
    conv_term = rho * uvec.transpose() * gradVec2(uvec, x, y)
    pressure_term = gradScalar2(p, x, y).transpose()
    source = conv_term + visc_term + pressure_term
    return source

def L_stokes_traction(uvec, p, x, y):
    mu, rho = sp.var('mu rho')
    visc_term = (-mu * divTen2(gradVec2(uvec, x, y) + gradVec2(uvec, x, y).transpose(), x, y)).transpose()
    pressure_term = gradScalar2(p, x, y).transpose()
    source = visc_term + pressure_term
    return source

def bc_terms_momentum_traction_no_turbulence(uvec, nvec, p, x, y, parts=True):
    mu, rho = sp.var('mu rho')
    visc_term = (-mu * nvec.transpose() * strain_rate(uvec, x, y)).transpose()
    if parts:
        pressure_term = (nvec.transpose() * eye2() * p).transpose()
    else:
        pressure_term = zeroVec2()
    return visc_term + pressure_term

def L_momentum_laplace(uvec, p, k, eps, x, y):
    cmu = 0.09
    mu, rho = sp.var('mu rho')
    visc_term = (-mu * divTen2(gradVec2(uvec, x, y), x, y)).transpose()
    conv_term = rho * uvec.transpose() * gradVec2(uvec, x, y)
    pressure_term = gradScalar2(p, x, y).transpose()
    turbulent_visc_term = -(divTen2(rho * cmu * k**2 / eps * (gradVec2(uvec, x, y)), x, y)).transpose()
    source = conv_term + visc_term + pressure_term + turbulent_visc_term
    return source

def L_momentum_laplace_no_turbulence(uvec, p, x, y):
    mu, rho = sp.var('mu rho')
    visc_term = (-mu * divTen2(gradVec2(uvec, x, y), x, y)).transpose()
    conv_term = rho * uvec.transpose() * gradVec2(uvec, x, y)
    pressure_term = gradScalar2(p, x, y).transpose()
    source = conv_term + visc_term + pressure_term
    return source

def L_stokes(uvec, p, x, y):
    mu, rho = sp.var('mu rho')
    visc_term = (-mu * divTen2(gradVec2(uvec, x, y), x, y)).transpose()
    pressure_term = gradScalar2(p, x, y).transpose()
    source = visc_term + pressure_term
    return source

def L_pressure(uvec, x, y):
    return -divVec2(uvec, x, y)

def L_kin(uvec, k, eps, x, y):
    cmu = 0.09
    sigk = 1.
    sigeps = 1.3
    c1eps = 1.44
    c2eps = 1.92
    conv_term = rho * uvec.transpose() * gradScalar2(k, x, y)
    diff_term = - divVec2((mu + rho * cmu * k**2 / eps / sigk) * gradScalar2(k, x, y), x, y)
    creation_term = - rho * cmu * k**2 / eps / 2 * strain_rate_squared_2(uvec, x, y)
    destruction_term = rho * eps
    terms = [conv_term[0,0], diff_term, creation_term, destruction_term]
    L = 0
    for term in terms:
        L += term
    return L

def L_eps(uvec, k, eps, x, y):
    cmu = 0.09
    sigk = 1.
    sigeps = 1.3
    c1eps = 1.44
    c2eps = 1.92
    conv_term = rho * uvec.transpose() * gradScalar2(eps, x, y)
    diff_term = - divVec2((mu + rho * cmu * k**2 / eps / sigeps) * gradScalar2(eps, x, y), x, y)
    creation_term = - rho * c1eps * cmu * k / 2 * strain_rate_squared_2(uvec, x, y)
    destruction_term = rho * c2eps * eps**2 / k
    terms = [conv_term[0,0], diff_term, creation_term, destruction_term]
    L = 0
    for term in terms:
        L += term
    return L

def L_coupled_gradient_source(v, x, y):
    return (-gradScalar2(v, x, y).transpose() * gradScalar2(v, x, y))[0,0]

def bc_terms_eps(nvec, k, eps, x, y):
    cmu = 0.09
    sigeps = 1.3
    mu, rho = sp.var('mu rho')
    return - nvec.transpose() * (mu + rho * cmu * k**2 / eps / sigeps) * gradScalar2(eps, x, y)

'''
Boundary condition operators
'''

def wall_function_momentum_traction(uvec, nvec, p, k, eps, x, y, tau_type, symbolic=True, parts=True):
    if symbolic:
        cmu = sp.var('c_{\mu}')
        yStarPlus = sp.var('y_{\mu}')
    else:
        cmu = 0.09
        yStarPlus = 11.06
    if tau_type == "vel":
        uvec_norm = sp.sqrt(uvec.transpose() * uvec)[0, 0]
        uTau = uvec_norm / yStarPlus
    elif tau_type == "kin":
        uTau = cmu**.25 * sp.sqrt(k)
    else:
        raise ValueError("Must either pass 'vel' or 'kin' for tau_type")

    mu, rho = sp.var('mu rho')
    normal_stress_term = (-nvec.transpose() * mu * strain_rate(uvec, x, y) * nvec * nvec.transpose()).transpose()
    tangential_stress_term = uTau / yStarPlus * uvec
    muT = rho * cmu * k * k / eps
    turbulent_stress_term = (-nvec.transpose() * muT * strain_rate(uvec, x, y)).transpose()
    if parts:
        pressure_term = (nvec.transpose() * eye2() * p).transpose()
    else:
        pressure_term = zeroVec2()
    return normal_stress_term + tangential_stress_term + turbulent_stress_term + pressure_term

def no_bc_bc(uvec, nvec, p, x, y, parts=True):
    mu, rho = sp.var('mu rho')
    visc_term = (-mu * nvec.transpose() * strain_rate(uvec, x, y)).transpose()
    import pdb; pdb.set_trace()
    if parts:
        pressure_term = (nvec.transpose() * eye2() * p).transpose()
    else:
        pressure_term = zeroVec2()
    return visc_term + pressure_term

def vacuum(u, nvec):
    return u / sp.Integer(2)

def ins_epsilon_wall_function_bc(nvec, k, eps, x, y):
    cmu = 0.09
    sigEps = 1.3
    kappa = 0.41
    mu, rho = sp.var('mu rho')
    muT = rho * cmu * k**2 / eps
    return - (mu + muT / sigEps) * kappa * cmu**.25 * sp.sqrt(k) * eps * rho / muT

def coupled_gradient_bc(nvec, v, x, y):
    return (-nvec.transpose() * gradScalar2(v, x, y))[0,0]

def coupled_value_bc(v, x, y):
    return -v

'''
Writing utilities
'''

def prep_moose_input(sym_expr):
    rep1 = re.sub(r'\*\*',r'^',str(sym_expr))
    rep2 = re.sub(r'mu',r'${mu}',rep1)
    rep3 = re.sub(r'rho',r'${rho}',rep2)
    rep4 = re.sub(r'ax', r'${ax}', rep3)
    rep5 = re.sub(r'ay', r'${ay}', rep4)
    return rep5
