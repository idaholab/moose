#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Solves a 3rd-order ODE associated with Jeffery-Hamel flow.
# See, for example: F. White, "Viscous Fluid Flow", pp. 168--172 for a
# derivation of the ODE. The 3rd-order ODE is recast as three
# first-order ODEs posed on the domain eta=(0,1).  Two of the solution
# components have initial data, but the third condition applies only
# at eta=1, so one has to iteratively attempt different initial
# condition values until one is found that give the proper final
# condition.  A Newton process can be used to converge the starting
# guess more quickly.  The ODEs to solve are:
#
# y0' = y1
# y1' = y2
# y2' = -2 * Re * alpha * y0 * y1 - 4 * alpha**2 * y1
#
# for 0 < eta < 1,
#
# With ICs:
# y0(0) = 1 (centerline velocity)
# y1(0) = 0 (centerline symmetry)
# y2(0) = guess
#
# where guess must be chosen iteratively such that y0(1)=0.
#
# Various things to consider:
# .) The step size in eta, "deta", must be small enough to guarantee accurate solution to the ODEs.
# .) The residual for the Newton iteration is: R(guess) = y0(1;guess)
# .) The derivative of this residual can be estimated by finite-differencing wrt guess.
# .) The Reynolds number is defined as: Re = (u_max * r * alpha) / nu
from scipy.integrate import ode
import matplotlib.pyplot as plt
import math
import numpy as np

# Use fonts that match LaTeX
from matplotlib import rcParams
rcParams['font.family'] = 'serif'
rcParams['font.size'] = 17
rcParams['font.serif'] = ['Computer Modern Roman']
rcParams['text.usetex'] = True

# Small font size for the legend
from matplotlib.font_manager import FontProperties
fontP = FontProperties()
fontP.set_size('x-small')

# Set up ODEs to be integrated.
def f(t, y, alpha, Re):
    return [y[1], y[2], -2 * Re * alpha * y[0] * y[1] - 4 * alpha**2 * y[1]]

# Set up Jacobian of the ODEs.
def jac(t, y, alpha, Re):
    return [[0, 1, 0],
            [0, 0, 1],
            [-2 * Re * alpha * y[1],
             -2 * Re * alpha * y[0] + 4 * alpha**2,
             0]]

def solve_ode(guess, dt, alpha, Re):
    '''
    guess - sets the initial condition for the y[2] component of the solution
    dt - the timestep size to use for the integration routine
    '''
    # Set initial conditions.
    y0, t0 = [1, 0, guess], 0

    # Construct ODE object.  I get an error when trying to use order=1
    # here. It seems like the default order is 5 if you specify nothing.
    # The possible method options is 'adams' and 'bdf'.  For this
    # problem, I saw no difference between 6th-order adams and 7-12th
    # order adams...
    r = ode(f, jac).set_integrator('vode', method='adams', with_jacobian=True, order=6)

    # Set up [alpha, Re] list of parameters which are used by the 'f' and 'jac' functions.
    p = [alpha, Re]

    # The ODE object does not store the entire solution internally.
    # That is up to the user.  In our case, we are interested in the
    # values of both y0 and y2.  y2 is needed for computing the exact
    # value of the pressure.
    timesteps = []
    solution = []
    second_deriv = []

    # This follows the various examples which call multiple functions on a single line
    r.set_initial_value(y0, t0).set_f_params(*p).set_jac_params(*p)

    # Store the initial condition for plotting later
    timesteps.append(t0)
    solution.append(y0[0])
    second_deriv.append(y0[2])

    # Do the time integration
    while r.successful() and abs(r.t - 1) > dt/2:
        r.integrate(r.t + dt)
        # For debugging purposes, we can print the most recent solution
        # print r.t, r.y
        timesteps.append(r.t)
        solution.append(r.y[0])
        second_deriv.append(r.y[2])

    # Return the ODE object as well as the computed solution at each time step.
    return r, timesteps, solution, second_deriv



################################################################################
# Main program
################################################################################

# The half-angle size of the wedge (in degrees)
alpha = 15 * math.pi/180

# The Reynolds number (Re = (u_max * r * alpha) / nu)
Re = 30

# Initialize Newton guess and counters
current_iterate = 0
max_its = 20
guess = 0.
nl_abs_tol = 1.e-13

# The timestep to use when integrating.  Overall, the solutions did
# not seem to be very sensitive to this choice.
# dt = .50
# dt = .25
# dt = .10
# dt = .05
# dt = .025
# dt = .0125
# dt = .00625
# dt = .003125
dt = .0015625
# dt = .00078125
# dt = .000390625
# dt = .0001953125
# dt = .00009765625
# dt = .000048828125
# dt = .0000244140625
# dt = .00001220703125
# dt = .000006103515625

# Finite differencing parameter used in approximating the Jacobian.
# Can't be too big or it will be inaccurate, can't be too small or it
# will get lost in roundoff error.  Might need to depend in some way
# on the size of the current residual, but luckily the parameter we
# are differencing with respect to is O(1).
newton_fd_eps = 1.e-8

# Control plotting/printing of f(theta)
print_results = True
plot_results = False

while (current_iterate < max_its):
    # print('Newton Iteration {}'.format(current_iterate))

    # Solve ODE system with current alpha value
    r, timesteps, solution, second_deriv = solve_ode(guess, dt, alpha, Re)

    # Compute residual for the current value of alpha
    newton_res = r.y[0]
    # print('  newton_res={}'.format(newton_res))

    # Print current solution, residual based on current value of alpha
    print('{:2d}: guess={:.15e}, |y0(eta=1)| = {:.8e}'.format(current_iterate, guess, abs(newton_res)))

    # If the Newton residual is small enough, break out of Newton loop
    if (abs(newton_res) < nl_abs_tol):
        break

    # We estimate Jacobian by finite differencing, so compute the ODE
    # with a perturbed initial guess.
    r2, dummy, dummy, dummy = solve_ode(guess + newton_fd_eps, dt, alpha, Re)

    # Approximate the Jacobian
    newton_jac = (r2.y[0] - r.y[0]) / newton_fd_eps
    # print('  newton_jac={}'.format(newton_jac))

    # Compute update
    dguess = -newton_res/newton_jac
    # print('  alpha={:.15e}'.format(alpha))

    # Update the parameter
    guess += dguess

    # Increment the Newton iterate
    current_iterate += 1


# Warn if we got here after using too many iterations.
if (current_iterate >= max_its):
    print('\nWarning, max iterates reached before reaching tolerance!\n')

# Compute the "constant" K for each value of eta.  It turns out it is
# not actually constant.. but it is fairly close.  We can compute a
# mean and standard deviation to try and pick the best single value...
K = []
for i in xrange(len(timesteps)):
    K.append(-1. / (4. * alpha * alpha) * (alpha * Re * solution[i]**2 + second_deriv[i]) - solution[i])

mean_K = np.mean(K)
std_K = np.std(K)

# Print mean and standard deviation of constant K.  The size of the
# standard deviation will certainly have some effect on the accuracy
# with which we can approximate the exact solution...
#
# For the alpha=15deg, Re=30 case, the results are:
# dt        mean(K)         std(K)
# .50,      -9.78221667283, 1.55432108496e-05
# .25,      -9.78221793235, 1.28909839468e-05
# .10,      -9.78221636352, 1.00149614255e-05 (max Newton iterations exceeded)
# .05,      -9.78221622393, 9.19157491110e-06
# .025,     -9.78222001829, 1.62242457073e-05
# .0125,    -9.78221345306, 7.36636294056e-06
# .00625,   -9.78221617295, 8.56617697780e-06
# .003125,  -9.78221785067, 7.05391749561e-06 (max Newton iterations exceeded)
# .0015625, -9.78221333616, 7.23063998586e-06
#
# Interestingly, neither the mean value of the constant nor the
# standard deviation seems to be affected by the eta discretization
# much.
print('mean(K) = {:.11f}, std(K) = {:.11e}'.format(mean_K, std_K))

# Print all results.  Useful for tabulating values of f(theta).
if print_results:
    print('eta'.rjust(22) + ', ' + 'f'.rjust(22) + ', ' + 'f"'.rjust(22))
    for i in xrange(len(timesteps)):
        print('{:.16e}, {:.16e}, {:.16e}, {:.16e}'.format(timesteps[i], solution[i], second_deriv[i], K[i]))

if plot_results:
    fig = plt.figure()
    ax1 = fig.add_subplot(111)
    ax1.plot(timesteps, solution, color="red", marker="o", linestyle="-", linewidth=2)
    ax1.set_xlabel(r'$\eta$')
    ax1.set_ylabel(r'$f(\eta)$')
    ax1.set_title(r'$\alpha={:.2f}^{{\circ}}$, $\mathrm{{Re}}={:.2f}$'.format(alpha*180/math.pi, Re))
    plt.savefig('jeffery_hamel.pdf', format='pdf')


# dt               alpha(degrees)   Re   f''(0)
# .05              15                1   -2.119080003201947e+00

# dt               alpha(degrees)   Re   f''(0)
# .05              15                5   -2.435444918589750e+00

# Results: f''(0) never really seems to converge to more than about 4
# digits?  Not sure how much this really affects the solution we get
# for f itself, perhaps not too much.
#
# dt               alpha(degrees)   Re   f''(0)
# .05              15               10   -2.891618720560730e+00
# .025             15               10   -2.891618642897614e+00
# .0125            15               10   -2.891618590770217e+00
# .00625           15               10   -2.891614175676399e+00
# .003125          15               10   -2.891620823937958e+00
# .0015625         15               10   -2.891620775691837e+00
# .00078125        15               10   -2.891620714390601e+00
# .000390625       15               10   -2.891620641141615e+00
# .0001953125      15               10   -2.891618831491945e+00
# .00009765625     15               10   -2.891618952886689e+00
# .000048828125    15               10   -2.891617481940699e+00
# .0000244140625   15               10   -2.891616474311833e+00
# .00001220703125  15               10   -2.891623890222196e+00
# .000006103515625 15               10   -2.891601195918375e+00 (|y0(1)| = 5.42448031e-15)

# dt               alpha(degrees)   Re   f''(0)
# .05              15               20   -4.022702806660169e+00

# dt               alpha(degrees)   Re   f''(0)
# .05              15               30   -5.446298538449031e+00

# For alpha=15 degrees, Re=40 is just beyond the point where flow
# "separation" would occur.  You can see that the boundary layer
# profile comes in to the wall almost vertically in the plot for this
# case.
#
# dt               alpha(degrees)   Re   f''(0)
# .05              15               40   -7.121324424148417e+00

# Local Variables:
# python-indent: 2
# End:
