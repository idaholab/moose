#!/usr/bin/env python
import mms
f = mms.evaluate('-div(grad(u))', 'sin(2*pi*x)*sin(2*pi*y)')
mms.print_fparser(f)

f = mms.evaluate('diff(u,t) - div(grad(u))', 't**3*x*y')
mms.print_fparser(f)
