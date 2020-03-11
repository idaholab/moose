#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script generates the solutions to the radially-symmetric cold-CO2 injection scenario
# presented in:
# (1) T LaForce, J Ennis-King, L Paterson ``Semi-analytical solutions for nonisothermal fluid injection including heat loss from the reservoir: Part 1. Saturation and temperature'' Advances in Water Resources 73 (2014) 227--234.
# (2)   T LaForce, A Mijic, J Ennis-King, L Paterson ``Semi-analytical solutions for nonisothermal fluid injection including heat loss from the reservoir: Part 2.  Pressure and stress'' Advances in Water Resources 73 (2014) 242--253.
#
# The main contolling parameter is "resolution", below.
# It is the number of points at which the output is computed at.
# The functions below are incredibly simple minded and involve integrations
# over the spatial domain, which means that this whole script completes in
# approximately resolution^2 time.
# resolution=10 gives a reasonable approximation, while resolution=800 is very good
#
# The other input parameters, such as fluid densities, etc, are all given below.

import os
import sys
import numpy as np
import scipy.optimize as opt
import scipy.interpolate
import matplotlib.pyplot as plt
import scipy.special


##############################
#                            #
# Main controlling parameter #
#                            #
##############################
resolution = 800


#######################
#                     #
# Physical quantities #
#                     #
#######################
rho1 = 970.00  # density of water
rho2 = 516.48  # density of CO2
C1 = 4149.0  # specific heat capacity of water
C2 = 2920.5  # specific heat capacity of CO2
phir = 0.2  # porosity of reservoir
phia = 0.02 # porosity of the adjacent formations
rhor = rhos = 2350.0  # density of the reservoir rock grains
rhoa = 2773.4  # density of adjacent formations
Cr = Cs = 1100.0  # specific heat capacity of reservoir rock grains
Ca = 828.9 # specific heat capacity of adjacent rock grains
tti = 358 # initial temperature of reservoir
ttw = 294 # injection temperature
ll = 5000.0  # lateral extent of reservoir
rw = 0.1 # radius of well
q = 15.855 # methane injection rate
h = 11.0  # height of reservoir
ss1r = 0.200 # residual saturation of water
ss2r = 0.205 # residual saturation of CO2
mu1_reservoir = 0.4704 # water viscosity at reservoir temperature
mu2_reservoir = 0.0163 # methane viscosity at reservoir temperature
mu1_intermediate = 0.6733 # water viscosity at intermediate temperature
mu2_intermediate = 0.0164 # methane viscosity at intermediate temperature
mu1_injection = 1.0473 # water viscosity at injection temperature
mu2_injection = 0.0167 # methane viscosity at injection temperature
mu1_moose = 3.394E-4 # viscosity of water used by MOOSE
mu2_moose = 3.93E-5 # viscosity of CO2 used by MOOSE
kka = 4.310 # wet thermal conductivity of the adjacent formation
comp1 = 1.0 / (2.27E14) # compressibility of water
k = 2E-12 # permeability in radial direction
thermal_expansion = 5E-6 # linear thermal expansion coefficient of rock
poisson = 0.2
young = 14.4E9
stress_eff_hor_ini = -12.8E6

# Tara parameters
#tti = 84.8
#ttw = 21
#mu2_moose = 0.0642E-3
#comp1 = 4.4E-10


######################
#                    #
# Derived quantities #
#                    #
######################
one_hour = 3600.0
one_day = one_hour * 24
one_month = one_day * 30
five_years = one_day * 365 * 5

alpha = rho2 * C2 / (rho1 * C1 - rho2 * C2)
beta = (rho2 * C2 + (1.0 - phir) * rhos * Cs / phir) / (rho1 * C1 - rho2 * C2)
gammaR = ll * ll * np.pi / (q / rho2) / (rho1 * C1 - rho2 * C2)
ttr = tti - ttw


##########
#        #
#  Code  #
#        #
##########
def integrate_over_r(rmin, rmax, fcn, n = 0, bias_to_small_r = True, num_rs = 100):
    # \int_{r=rmin}^{r=rmax}dr r^{n} fcn
    # fcn should be a scipy.interpolate.interp1d object
    # This uses the trapezoidal rule
    # If bias_to_small_r=true then the "r points" are clustered more closely around r=rmin

    if bias_to_small_r:
        r_points = [np.log10(rmin) + (np.log10(rmax) - np.log10(rmin)) * i / float(num_rs - 1) for i in range(num_rs)]
        r_points = [np.power(10, x) for x in r_points]
    else:
        r_points = [rmin + (rmax - rmin) * i / float(num_rs - 1) for i in range(num_rs)]

    result = 0.0
    for i in range(num_rs - 1):
        a = r_points[i]
        b = r_points[i + 1]
        result += 0.5 * (b - a) * (np.power(b, n) * fcn(b) + np.power(a, n) * fcn(a))

    return result


def xd_given_r(r):
    return r * r / ll / ll

def r_given_xd(xd):
    return np.sqrt(xd) * ll

def td_given_t(t):
    return (q / rho2) * t / np.pi / phir / ll / ll / h

def t_given_td(td):
    return td * np.pi * phir * ll * ll * h / (q / rho2)

def uu_given_t(t):
    return np.sqrt(kka * (phia * rho1 * C1 + (1.0 - phia) * rhoa * Ca) / t)

def ttd_given_tt(tt):
    return (tt - tti) / (ttw - tti)

def tt_given_ttd(ttd):
    return ttd * (ttw - tti) + tti

def sshat_given_ss1(ss1):
    return (ss1 - ss1r) / (1 - ss1r - ss2r)

def dsshat_dss1():
    return 1.0 / (1 - ss1r - ss2r)

def ss1_given_sshat(sshat):
    return (1 - ss1r - ss2r) * sshat + ss1r

def kr1_hat(sshat):
    return np.power(sshat, 4.0)

def dkr1_hat_dsshat(sshat):
    return 4.0 * np.power(sshat, 3.0)

def kr2_hat(sshat):
    return np.power(1 - sshat, 2.0) * (1 - sshat * sshat)

def dkr2_hat_dsshat(sshat):
    return -4 * np.power(sshat, 3) + 6 * np.power(sshat, 2) - 2

def kr1(ss1):
    sshat = sshat_given_ss1(ss1)
    if sshat <= 0:
        return 0.0
    elif sshat >= 1:
        return 1.0
    return kr1_hat(sshat)

def dkr1_dss1(ss1):
    sshat = sshat_given_ss1(ss1)
    if sshat <= 0:
        return 0.0
    elif sshat >= 1:
        return 0.0
    return dkr1_hat_dsshat(sshat) * dsshat_dss1()

def kr2(ss1):
    sshat = sshat_given_ss1(ss1)
    if sshat <= 0:
        return 1.0
    elif sshat >= 1:
        return 0.0
    return kr2_hat(sshat)

def dkr2_dss1(ss1):
    sshat = sshat_given_ss1(ss1)
    if sshat <= 0:
        return 0.0
    elif sshat >= 1:
        return 0.0
    return dkr2_hat_dsshat(sshat) * dsshat_dss1()

def f1(ss1, mu1, mu2):
    sshat = sshat_given_ss1(ss1)
    if sshat <= 0:
        return 0.0
    elif sshat >= 1:
        return 1.0
    return 1.0 / (1.0 + kr2(ss1) * mu1 / kr1(ss1) / mu2)

def df1_dss1(ss1, mu1, mu2):
    sshat = sshat_given_ss1(ss1)
    if sshat <= 0:
        return 0.0
    elif sshat >= 1:
        return 0.0
    dby_denom = - np.power(f1(ss1, mu1, mu2), 2)
    return dby_denom * (mu1 / mu2) * (dkr2_dss1(ss1) / kr1(ss1) - kr2(ss1) * dkr1_dss1(ss1) / np.power(kr1(ss1), 2))

def f2(ss1, mu1, mu2):
    return 1.0 - f1(ss1, mu1, mu2)

def df2_dss1(ss1, mu1, mu2):
    return - df1_dss1(ss1, mu1, mu2)

def ss1ss(mu1, mu2):
    # returns S_{1S}, which is the water saturation when
    # df1_dss1(S) = (f1(S) - 1) / (S - 1)
    def f(s):
        return df1_dss1(s, mu1, mu2) - (f1(s, mu1, mu2) - 1) / (s - 1)
    return opt.fsolve(f, 0.6)[0]

def ss1tt(mu1, mu2):
    # returns S_{1T}, which is the water saturation when
    # df1_dss1(S) = (f1(S) + alpha) / (S + beta)
    def f(s):
        return df1_dss1(s, mu1, mu2) - (f1(s, mu1, mu2) + alpha) / (s + beta)
    return opt.fsolve(f, 0.4)[0]


# Tara's S_{1S} in upper-left Fig2 of paper1
ss1ss_reservoir = ss1ss(mu1_reservoir, mu2_reservoir)
ss1ss_injection = ss1ss(mu1_injection, mu2_injection)
ss1ss_moose = ss1ss(mu1_moose, mu2_moose)

def saturation_shock_position(t, mu1, mu2):
    td = td_given_t(t)
    v_saturation_shock = df1_dss1(ss1ss(mu1, mu2), mu1, mu2)
    xd = v_saturation_shock * td
    return r_given_xd(xd)

def temperature_shock_position(t, mu1, mu2):
    td = td_given_t(t)
    v_temperature_shock = df1_dss1(ss1tt(mu1, mu2), mu1, mu2)
    xd = v_temperature_shock * td
    return r_given_xd(xd)

def co2_saturation(t, mu1, mu2, num_saturations = 1000):
    # co2 saturation as a function of radial position
    local_ss1ss = ss1ss(mu1, mu2)
    water_saturations = [local_ss1ss * i / float(num_saturations - 1) for i in range(num_saturations)]
    velocities = [df1_dss1(s, mu1, mu2) for s in water_saturations]
    td = td_given_t(t)
    xds = [v * td for v in velocities]
    r = [r_given_xd(xd) for xd in xds]
    r += [r[-1] + 1E-9, ll]
    co2_saturations = [1 - s for s in water_saturations] + [0, 0]
    return (r, co2_saturations)

def tt_result(t, mu1, mu2, num_rs = 10):
    # temperature distribution as a function of radial position

    shock_pos = temperature_shock_position(t, mu1, mu2)
    rs = [shock_pos * i / float(num_rs - 1) for i in range(num_rs)]
    xds = [xd_given_r(r) for r in rs]

    # calculate the water saturation
    co2_s = co2_saturation(t, mu1, mu2, num_rs)
    xds_where_s_defined = [xd_given_r(r_val) for r_val in co2_s[0]]
    s_water = [(1 - s_val) for s_val in co2_s[1]]
    while (xds_where_s_defined[0] == 0.0):
        xds_where_s_defined.pop(0)
        s_water.pop(0)
    xds_where_s_defined.insert(0, 0.0)
    s_water.insert(0, ss1r)
    s_water_interp = scipy.interpolate.interp1d(xds_where_s_defined, s_water)

    # calculate integral of Eqn(20) of Tara's first paper, for each xd in xds
    integral = []
    for xd in xds:
        finite_difference_points = [xd * i / float(num_rs - 1) for i in range(num_rs)]
        integrand = [-gammaR * uu_given_t(t) / (f1(s_water_interp(xd), mu1, mu2) + alpha) for xd in xds]
        integral.append(sum(integrand) * (finite_difference_points[1] - finite_difference_points[0])) # edges of integral probably don't matter if num_rs is large enough

    ttd_vals = np.exp(integral)
    tt_vals = [tt_given_ttd(ttd) for ttd in ttd_vals]
    rs += [rs[-1] * (1 + 1E-9), ll]
    tt_vals += [tti, tti]
    return (rs, tt_vals)

def ggp(t, mu1, mu2, num_rs = 10, rmin = rw):
    # Gp(r) of Eqn(14) in Tara's second paper (i've written it as a fcn of (t, r) in contrast to Tara's (td, rd))

    shock_pos = saturation_shock_position(t, mu1, mu2)

    # rs and xds are the positions to provide the answer (Gp) at
    rs = [np.log10(rmin) + (np.log10(shock_pos)- np.log10(rmin)) * i / float(num_rs - 1) for i in range(num_rs)]
    rs = [np.power(10, r) for r in rs]

    # calculate the water saturation
    co2_s = co2_saturation(t, mu1, mu2, num_rs)
    r_vals_where_s_defined = co2_s[0]
    s_water = [(1 - s_val) for s_val in co2_s[1]]
    while r_vals_where_s_defined[0] == 0.0:
        r_vals_where_s_defined.pop(0)
        s_water.pop(0)
    r_vals_where_s_defined.insert(0, 0.0)
    s_water.insert(0, ss1r)
    s_water_interp = scipy.interpolate.interp1d(r_vals_where_s_defined, s_water)

    # calculate integral of Eqn(14) of Tara's first paper, for each xd in xds
    integral = []
    for r in rs:
        r_points = [np.log10(r) + (np.log10(rs[-1]) - np.log10(r)) * i / float(num_rs - 1) for i in range(num_rs)]
        r_points = [np.power(10, x) for x in r_points]
        this_integral = 0.0
        for i in range(len(r_points) - 1):
            a = r_points[i]
            b = r_points[i + 1]
            # the 2.0 in the following line comes from dr_D / r_D = 2.0 dr / r
            this_integral += 0.5 * (b - a) * (2.0 / b / (kr1(s_water_interp(b)) / mu1 + kr2(s_water_interp(b)) / mu2) + 2.0 / a / (kr1(s_water_interp(a)) / mu1 + kr2(s_water_interp(a)) / mu2))
        integral.append(this_integral)

    return (rs, integral)

def ggb(t, r, mu1):
    # Gb of Eqn(15) in Tara's second paper (i've written it as a fcn of (t, r) in contrast to Tara's (td, rd))
    return - scipy.special.expi(- (q / rho2) * mu1 * comp1 * xd_given_r(r) / 4 / np.pi / h / k / td_given_t(t))

def ggll(t, r, mu1):
    # G_L of Eqn(16) in Tara's second paper (i've written it as a fcn of (t, r) in contrast to Tara's (td, rd))
    return - scipy.special.expi(- (q / rho2) * mu1 * comp1 * xd_given_r(ll) / 4 / np.pi / h / k / td_given_t(t))

def pp_increase(t, mu1, mu2, num_rs = 10, rmin = rw):

    shock_pos = saturation_shock_position(t, mu1, mu2)
    rs = [np.log10(rmin) + (np.log10(ll)- np.log10(rmin)) * i / float(num_rs - 1) for i in range(num_rs)]
    rs = [np.power(10, r) for r in rs]

    precompute_ggp = ggp(t, mu1, mu2, num_rs)
    ggp_interp = scipy.interpolate.interp1d(precompute_ggp[0], precompute_ggp[1])

    pp_inc = []
    for r in rs:
        if r <= shock_pos:
            pp_inc.append(ggp_interp(r) + mu1 * ggb(t, shock_pos, mu1) - mu1 * ggll(t, r, mu1))
        else:
            pp_inc.append(mu1 * ggb(t, r, mu1) - mu1 * ggll(t, r, mu1))

    return rs, [(q / rho2) / 4 / np.pi / h / k * delp for delp in pp_inc]


def upr(t, temp_soln, pp_inc, num_rs = 10, rmin = rw):
    # u_pr in Eqn(33) of Tara's second paper
    rs = [np.log10(rmin) + (np.log10(ll)- np.log10(rmin)) * i / float(num_rs - 1) for i in range(num_rs)]
    rs = [np.power(10, r) for r in rs]

    # first do the integral of g = T_{D2} (Eqn(34))
    temp_interp = scipy.interpolate.interp1d(temp_soln[0], temp_soln[1])
    g1 = [0.0]
    for r in rs[1:]:
        r_points = [np.log10(rmin) + (np.log10(r) - np.log10(rmin)) * i / float(num_rs - 1) for i in range(num_rs)]
        r_points = [np.power(10, x) for x in r_points]
        this_integral = 0.0
        for i in range(len(r_points) - 1):
            a = r_points[i]
            b = r_points[i + 1]
            # the (1.0 / rw^2) in the following line comes from eta deta = r dr / rw^2
            # T_{D2} = -T_{D1} = (T - T_{i})/ (T_{w} - T_{i})
            # NOTE: check this negative sign
            contrib = - 0.5 * (b - a) * (b * ttd_given_tt(temp_interp(b)) + a * ttd_given_tt(temp_interp(a)))
            this_integral += contrib * (1.0 / rw / rw)
        g1.append(this_integral)

    # second do the integral of g = (1 - 2v)/(1 - v) P_{D}
    pp_interp = scipy.interpolate.interp1d(pp_inc[0], pp_inc[1])
    g2 = [0.0]
    for r in rs[1:]:
        r_points = [np.log10(rmin) + (np.log10(r) - np.log10(rmin)) * i / float(num_rs - 1) for i in range(num_rs)]
        r_points = [np.power(10, x) for x in r_points]
        this_integral = 0.0
        for i in range(len(r_points) - 1):
            a = r_points[i]
            b = r_points[i + 1]
            # the (1.0 / rw^2) in the following line comes from eta deta = r dr / rw^2
            contrib = 0.5 * (b - a) * (b * pp_interp(b) + a * pp_interp(a))
            this_integral += contrib * (1.0 / rw / rw) * (1 - 2 * poisson) / thermal_expansion / ttr / young
        g2.append(this_integral)

    g = [(g1[i] + g2[i]) * rw / rs[i] for i in range(len(rs))]
    return rs, g

def aa_bb(upr_soln, pp_inc):
    # Computes A and B, or Eqn(A2) of Tara's second paper

    # ASSUME that pp_inc[1][0] is the porepressure increase at the well
    phatzero = (pp_inc[1][0] + stress_eff_hor_ini)* (1 - poisson) / thermal_expansion / ttr / young  # Probably tara had -stress_eff_hor_ini instead as that gives similar displacements, but incorrect effective stresses near the origin
    # ASSUME that upr_soln[1][-1] = u_{pr}(xi=xi1), ie, that upr has been evaluated up to xi=xi1 (which is r=L)
    bb = ((1 - 2 * poisson) * phatzero - upr_soln[1][-1] * rw / ll) / (1 - 2 * poisson + rw * rw / ll / ll) # Eqn (A2) of Tara's second paper (possibly with fixed sign) with fixed denominator (that was wrong in Rehbinder too)
    aa = -bb * rw * rw / ll / ll - upr_soln[1][-1] * rw / ll # Eqn (A2) of Tara's second paper with fixed sign on first term
    return (aa, bb)

def uh(upr_soln, pp_inc, num_rs = 10, rmin = rw):
    # u_h in Eqn(32) of Tara's second paper
    aa, bb = aa_bb(upr_soln, pp_inc)

    rs = [np.log10(rmin) + (np.log10(ll)- np.log10(rmin)) * i / float(num_rs - 1) for i in range(num_rs)]
    rs = [np.power(10, r) for r in rs]

    result = []
    for r in rs:
        result.append(aa * r / rw + bb * rw / r)

    return (rs, result)


def eff_rr(upr_soln, pp_inc, num_rs = 10, rmin = rw):
    # effective stress in rr direction: Eqn(A3) of Tara's second paper
    aa, bb = aa_bb(upr_soln, pp_inc)

    if pp_inc[0] != upr_soln[0]:
        sys.stderr.write("pp_inc defined at different points to upr_soln\n")
        sys.exit(1)

    result = [stress_eff_hor_ini + (thermal_expansion * ttr * young / (1 - poisson)) * (aa / (1 - 2 * poisson) - bb * rw * rw / pp_inc[0][i] / pp_inc[0][i] + (1 - poisson) * pp_inc[1][i] / thermal_expansion / ttr / young - upr_soln[1][i] * rw / pp_inc[0][i]) for i in range(len(pp_inc[0]))]

    return pp_inc[0], [r / 1E6 for r in result]



def eff_tt(upr_soln, pp_inc, tt_soln, num_rs = 10, rmin = rw):
    # effective stress in tt direction: Eqn(A3) of Tara's second paper
    aa, bb = aa_bb(upr_soln, pp_inc)

    if pp_inc[0] != upr_soln[0]:
        sys.stderr.write("pp_inc defined at different points to upr_soln\n")
        sys.exit(1)

    tt_interp = scipy.interpolate.interp1d(tt_soln[0], tt_soln[1])
    result = [stress_eff_hor_ini + (thermal_expansion * ttr * young / (1 - poisson)) * (aa / (1 - 2 * poisson) + bb * rw * rw / pp_inc[0][i] / pp_inc[0][i] + pp_inc[1][i] * poisson / thermal_expansion / ttr / young + upr_soln[1][i] * rw / pp_inc[0][i] + ttd_given_tt(tt_interp(pp_inc[0][i]))) for i in range(len(pp_inc[0]))]

    return pp_inc[0], [r / 1E6 for r in result]



def save_to_file(file_name, data):
    f = open(file_name, 'w')
    trans = zip(*data)
    for d in trans:
        f.write(",".join(map(str, d)) + "\n")
    f.close()


######################################
#                                    #
# Generate results and save to files #
#                                    #
######################################

plt.figure()
pp_increase_one_hour = pp_increase(one_hour, mu1_moose, mu2_moose, num_rs=resolution)
pp_increase_one_day = pp_increase(one_day, mu1_moose, mu2_moose, num_rs=resolution)
pp_increase_one_month = pp_increase(one_month, mu1_moose, mu2_moose, num_rs=resolution)
pp_increase_five_years = pp_increase(five_years, mu1_moose, mu2_moose, num_rs=resolution)
plt.semilogx(pp_increase_one_hour[0], [p / 1E6 for p in pp_increase_one_hour[1]], 'b-', label = '1 hour')
plt.semilogx(pp_increase_one_day[0], [p / 1E6 for p in pp_increase_one_day[1]], 'r-', label = '1 day')
plt.semilogx(pp_increase_one_month[0], [p / 1E6 for p in pp_increase_one_month[1]], 'g-', label = '1 month')
plt.semilogx(pp_increase_five_years[0], [p / 1E6 for p in pp_increase_five_years[1]], 'k-', label = '5 years')
plt.legend(loc = 'best')
plt.xlim([0.1, ll])
plt.xlabel("r (m)")
plt.ylabel("Porepressure increase (MPa)")
plt.title("Porepressure")
#plt.savefig("pp.pdf")
save_to_file("pp_one_hour.csv", pp_increase_one_hour)
save_to_file("pp_one_day.csv", pp_increase_one_day)
save_to_file("pp_one_month.csv", pp_increase_one_month)
save_to_file("pp_five_years.csv", pp_increase_five_years)



plt.figure()
sg_one_hour = co2_saturation(one_hour, mu1_moose, mu2_moose, num_saturations=resolution)
sg_one_day = co2_saturation(one_day, mu1_moose, mu2_moose, num_saturations=resolution)
sg_one_month = co2_saturation(one_month, mu1_moose, mu2_moose, num_saturations=resolution)
sg_five_years = co2_saturation(five_years, mu1_moose, mu2_moose, num_saturations=resolution)
plt.semilogx(sg_one_hour[0], sg_one_hour[1], 'b-', label = '1 hour')
plt.semilogx(sg_one_day[0], sg_one_day[1], 'r-', label = '1 day')
plt.semilogx(sg_one_month[0], sg_one_month[1], 'g-', label = '1 month')
plt.semilogx(sg_five_years[0], sg_five_years[1], 'k-', label = '5 years')
plt.legend(loc = 'best')
plt.xlim([0.1, ll])
plt.xlabel("r (m)")
plt.ylabel("Saturation")
plt.title("CO2 saturation")
#plt.savefig("sg.pdf")
save_to_file("sg_one_hour.csv", sg_one_hour)
save_to_file("sg_one_day.csv", sg_one_day)
save_to_file("sg_one_month.csv", sg_one_month)
save_to_file("sg_five_years.csv", sg_five_years)



plt.figure()
tt_one_hour = tt_result(one_hour, mu1_moose, mu2_moose, num_rs=resolution)
tt_one_day = tt_result(one_day, mu1_moose, mu2_moose, num_rs=resolution)
tt_one_month = tt_result(one_month, mu1_moose, mu2_moose, num_rs=resolution)
tt_five_years = tt_result(five_years, mu1_moose, mu2_moose, num_rs=resolution)
plt.semilogx(tt_one_hour[0], tt_one_hour[1], 'b-', label = '1 hour')
plt.semilogx(tt_one_day[0], tt_one_day[1], 'r-', label = '1 day')
plt.semilogx(tt_one_month[0], tt_one_month[1], 'g-', label = '1 month')
plt.semilogx(tt_five_years[0], tt_five_years[1], 'k-', label = '5 years')
plt.legend(loc = 'best')
plt.xlim([0.1, ll])
plt.xlabel("r (m)")
plt.ylabel("Temperature (K)")
plt.title("Temperature")
#plt.savefig("temp.pdf")
save_to_file("tt_one_hour.csv", tt_one_hour)
save_to_file("tt_one_day.csv", tt_one_day)
save_to_file("tt_one_month.csv", tt_one_month)
save_to_file("tt_five_years.csv", tt_five_years)


plt.figure()
upr_one_hour = upr(one_hour, tt_one_hour, pp_increase_one_hour, num_rs=resolution, rmin = rw)
uh_one_hour = uh(upr_one_hour, pp_increase_one_hour, num_rs=resolution, rmin=rw)
uhat_one_hour = [upr_one_hour[1][i] + uh_one_hour[1][i] for i in range(len(upr_one_hour[1]))]
u_one_hour = (uh_one_hour[0], [1E3 * thermal_expansion * ttr * rw * (1 + poisson) / (1 - poisson) * uhat for uhat in uhat_one_hour])
upr_one_day = upr(one_day, tt_one_day, pp_increase_one_day, num_rs=resolution, rmin = rw)
uh_one_day = uh(upr_one_day, pp_increase_one_day, num_rs=resolution, rmin=rw)
uhat_one_day = [upr_one_day[1][i] + uh_one_day[1][i] for i in range(len(upr_one_day[1]))]
u_one_day = (uh_one_day[0], [1E3 * thermal_expansion * ttr * rw * (1 + poisson) / (1 - poisson) * uhat for uhat in uhat_one_day])
upr_one_month = upr(one_month, tt_one_month, pp_increase_one_month, num_rs=resolution, rmin = rw)
uh_one_month = uh(upr_one_month, pp_increase_one_month, num_rs=resolution, rmin=rw)
uhat_one_month = [upr_one_month[1][i] + uh_one_month[1][i] for i in range(len(upr_one_month[1]))]
u_one_month = (uh_one_month[0], [1E3 * thermal_expansion * ttr * rw * (1 + poisson) / (1 - poisson) * uhat for uhat in uhat_one_month])
upr_five_years = upr(five_years, tt_five_years, pp_increase_five_years, num_rs=resolution, rmin = rw)
uh_five_years = uh(upr_five_years, pp_increase_five_years, num_rs=resolution, rmin=rw)
uhat_five_years = [upr_five_years[1][i] + uh_five_years[1][i] for i in range(len(upr_five_years[1]))]
u_five_years = (uh_five_years[0], [1E3 * thermal_expansion * ttr * rw * (1 + poisson) / (1 - poisson) * uhat for uhat in uhat_five_years])
plt.semilogx(u_one_hour[0], u_one_hour[1], 'b-', label = '1 hour')
plt.semilogx(u_one_day[0], u_one_day[1], 'r-', label = '1 day')
plt.semilogx(u_one_month[0], u_one_month[1], 'g-', label = '1 month')
plt.semilogx(u_five_years[0], u_five_years[1], 'k-', label = '5 years')
plt.legend(loc = 'best')
plt.xlim([0.1, ll])
plt.xlabel("r (m)")
plt.ylabel("Displacement (mm)")
plt.title("Radial displacement")
#plt.savefig("u.pdf")
save_to_file("u_one_hour.csv", u_one_hour)
save_to_file("u_one_day.csv", u_one_day)
save_to_file("u_one_month.csv", u_one_month)
save_to_file("u_five_years.csv", u_five_years)

seff_rr_one_hour = eff_rr(upr_one_hour, pp_increase_one_hour, num_rs=resolution, rmin=rw)
seff_tt_one_hour = eff_tt(upr_one_hour, pp_increase_one_hour, tt_one_hour, num_rs=resolution, rmin=rw)
seff_rr_one_day = eff_rr(upr_one_day, pp_increase_one_day, num_rs=resolution, rmin=rw)
seff_tt_one_day = eff_tt(upr_one_day, pp_increase_one_day, tt_one_day, num_rs=resolution, rmin=rw)
seff_rr_one_month = eff_rr(upr_one_month, pp_increase_one_month, num_rs=resolution, rmin=rw)
seff_tt_one_month = eff_tt(upr_one_month, pp_increase_one_month, tt_one_month, num_rs=resolution, rmin=rw)
seff_rr_five_years = eff_rr(upr_five_years, pp_increase_five_years, num_rs=resolution, rmin=rw)
seff_tt_five_years = eff_tt(upr_five_years, pp_increase_five_years, tt_five_years, num_rs=resolution, rmin=rw)

plt.figure()
plt.semilogx(seff_rr_one_hour[0], seff_rr_one_hour[1], 'b-', label = '1 hour')
plt.semilogx(seff_rr_one_day[0], seff_rr_one_day[1], 'r-', label = '1 day')
plt.semilogx(seff_rr_one_month[0], seff_rr_one_month[1], 'g-', label = '1 month')
plt.semilogx(seff_rr_five_years[0], seff_rr_five_years[1], 'k-', label = '5 years')
plt.legend(loc = 'best')
plt.xlim([0.1, ll])
plt.xlabel("r (m)")
plt.ylabel("Stress (MPa)")
plt.title("Effective radial stress")
#plt.savefig("seff_rr.pdf")
save_to_file("seff_rr_one_hour.csv", seff_rr_one_hour)
save_to_file("seff_rr_one_day.csv", seff_rr_one_day)
save_to_file("seff_rr_one_month.csv", seff_rr_one_month)
save_to_file("seff_rr_five_years.csv", seff_rr_five_years)

plt.figure()
plt.semilogx(seff_tt_one_hour[0], seff_tt_one_hour[1], 'b-', label = '1 hour')
plt.semilogx(seff_tt_one_day[0], seff_tt_one_day[1], 'r-', label = '1 day')
plt.semilogx(seff_tt_one_month[0], seff_tt_one_month[1], 'g-', label = '1 month')
plt.semilogx(seff_tt_five_years[0], seff_tt_five_years[1], 'k-', label = '5 years')
plt.legend(loc = 'best')
plt.xlim([0.1, ll])
plt.xlabel("r (m)")
plt.ylabel("Stress (MPa)")
plt.title("Effective hoop stress")
#plt.savefig("seff_tt.pdf")
save_to_file("seff_tt_one_hour.csv", seff_tt_one_hour)
save_to_file("seff_tt_one_day.csv", seff_tt_one_day)
save_to_file("seff_tt_one_month.csv", seff_tt_one_month)
save_to_file("seff_tt_five_years.csv", seff_tt_five_years)

sys.exit(0)
