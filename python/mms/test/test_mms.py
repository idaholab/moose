#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import unittest
import mms

class TestMMS(unittest.TestCase):

    def testBasic(self):
        f,s = mms.evaluate('div(grad(u))', 'x**3')
        fs = mms.fparser(f)
        self.assertEqual(fs, '6*x')

        ss = mms.fparser(s)
        self.assertEqual(ss, 'x^3')

    def testEvaluate(self):
        f,_ = mms.evaluate('diff(h, t) + div(u*h) + div(grad(r*h))',
                         'cos(x*y*t)', variable='h', scalars=['r'], vectors=['u'])

        s = mms.fparser(f)
        self.assertEqual(s, '-x^2*r*t^2*cos(x*y*t) - x*y*sin(x*y*t) - x*t*u_y*sin(x*y*t) - ' \
                            'y^2*r*t^2*cos(x*y*t) - y*t*u_x*sin(x*y*t)')

        s = mms.moosefunction(f)
        self.assertEqual(s, '-std::pow(p(0), 2)*_r*std::pow(t, 2)*std::cos(p(0)*p(1)*t) - ' \
                            'p(0)*p(1)*std::sin(p(0)*p(1)*t) - p(0)*t*_u(1)*std::sin(p(0)*p(1)*t) ' \
                            '- std::pow(p(1), 2)*_r*std::pow(t, 2)*std::cos(p(0)*p(1)*t) - ' \
                            'p(1)*t*_u(0)*std::sin(p(0)*p(1)*t)')

    def testCylindricalEvaluate(self):
        f,_ = mms.evaluate('div(u)', 'r*phi*z*(e_i+e_j+e_k)', transformation='cylindrical',
                           coordinate_names=('r','phi','z'))
        s = mms.fparser(f)

        self.assertEqual(s, 'phi*r + 2*phi*z + z')

    def testEvaluateWithScalarFunction(self):
        f, _ = mms.evaluate('diff(h*u, t)', 'cos(x*t)', functions=['h'])
        s = mms.fparser(f)

        self.assertEqual(s, '-x*h(R.x, R.y, R.z, t)*sin(x*t) + ' \
                            'cos(x*t)*Derivative(h(R.x, R.y, R.z, t), t)')

    def testEvaluateWithVectorFunction(self):
        f, _ = mms.evaluate('div(h*u)', 'cos(x*t)', vectorfunctions=['h'])
        s = mms.fparser(f)

        self.assertEqual(s, '-t*h_x(R.x, R.y, R.z, t)*sin(x*t) + ' \
                            'cos(x*t)*Derivative(h_x(R.x, R.y, R.z, t), R.x) + ' \
                            'cos(x*t)*Derivative(h_y(R.x, R.y, R.z, t), R.y) + ' \
                            'cos(x*t)*Derivative(h_z(R.x, R.y, R.z, t), R.z)')

    def testEvaluateWithKwargs(self):
        f, _ = mms.evaluate('div(h*u)', 'cos(x*t)*e_i', scalars=['k'], h='k*x*x')
        s = mms.fparser(f)

        self.assertEqual(s, '-x^2*k*t*sin(x*t) + 2*x*k*cos(x*t)')

    def testEvaluateVectorFunction(self):
        f, e = mms.evaluate('div(u.outer(u))', 'cos(x*t)*e_i')

        s = mms.fparser(f)
        self.assertEqual(s, '[-2*t*sin(x*t)*cos(x*t), 0, 0]')

        s = mms.fparser(e)
        self.assertEqual(s, '[cos(x*t), 0, 0]')

    def testExceptions(self):

        try:
            mms.evaluate('div(h*u)', 'cos(x*t)*e_i', scalars=['R'], h='k*x*x')
        except SyntaxError as e:
            self.assertIn("name 'R'", str(e))

        try:
            mms.evaluate('div(h*u)', 'cos(x*t)*e_i', scalars=['x'], h='k*x*x')
        except SyntaxError as e:
            self.assertIn("name 'x'", str(e))

        try:
            mms.evaluate('div(h*u)', 'cos(x*t)*e_i', scalars=['t'], h='k*x*x')
        except SyntaxError as e:
            self.assertIn("name 't'", str(e))

        try:
            mms.evaluate('div(h*u)', 'cos(x*t)*e_i', scalars=['e_k'], h='k*x*x')
        except SyntaxError as e:
            self.assertIn("name 'e_k'", str(e))

    def testHit(self):
        f,s = mms.evaluate('a*div(k*grad(u))', 'x**3', scalars=['k', 'a'])
        n = mms.build_hit(f, 'force', a=42).render()

        self.assertIn('[force]', n)
        self.assertIn('type = ParsedFunction', n)
        self.assertIn("value = '6*x*a*k'", n)
        self.assertIn("vars = 'a k'", n)
        self.assertIn("vals = '42 1.0'", n)


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
