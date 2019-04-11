#!/usr/bin/env python
import mms
fs,ss = mms.evaluate('-div(grad(u))', 'sin(2*pi*x)*sin(2*pi*y)')
mms.print_fparser(fs)

mms.print_hit(fs, 'force')
mms.print_hit(ss, 'exact')

ft,st = mms.evaluate('diff(u,t) - div(grad(u))', 't**3*x*y')
mms.print_fparser(ft)

mms.print_hit(ft, 'force')
mms.print_hit(st, 'exact')
