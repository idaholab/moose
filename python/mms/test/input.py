#!/usr/bin/env python
import mms

f = mms.evaluate('div(grad(u))', 'a*sin(2*pi*a*x*y)', scalars='a')
mms.print_fparser(f)

mms.plot('input_out.csv', output='convergence.pdf', show=False)
