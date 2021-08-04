#!/usr/bin/env python3

# MooseDocs:start:spatial
import mms
fs, ss = mms.evaluate(('rho * cp * diff(u,t) - div(k*grad(u)) - '
                      'shortwave*sin(0.5*x*pi)*exp(kappa*y)*sin(1/(hours*3600)*pi*t)'),
                      't*sin(pi*x)*sin(5*pi*y)',
                      scalars=['rho', 'cp', 'k', 'kappa', 'shortwave', 'hours'])
mms.print_hit(fs, 'mms_force')
mms.print_hit(ss, 'mms_exact')
# MooseDocs:end:spatial

# MooseDocs:start:temporal
import mms
fs, ss = mms.evaluate(('rho * cp * diff(u,t) - div(k*grad(u)) - '
                      'shortwave*sin(0.5*x*pi)*exp(kappa*y)*sin(1/(hours*3600)*pi*t)'),
                      'x*y*exp(-1/32400*t)',
                      scalars=['rho', 'cp', 'k', 'kappa', 'shortwave', 'hours'])
mms.print_hit(fs, 'mms_force')
mms.print_hit(ss, 'mms_exact')
# MooseDocs:end:temporal
