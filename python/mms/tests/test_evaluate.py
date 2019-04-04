#!/usr/bin/env python
import unittest
import mms

class TestMMSEvaluate(unittest.TestCase):
    def test(self):
        f = mms.evaluate('diff(h, t) + div(u*h) + div(grad(r*h))',
                         'cos(x*y*t)', variable='h', scalars='r', vectors='u')

        s = mms.fparser(f)
        self.assertEqual(s, '-x^2*r*t^2*cos(x*y*t) - x*y*sin(x*y*t) - x*t*u_y*sin(x*y*t) - ' \
                            'y^2*r*t^2*cos(x*y*t) - y*t*u_x*sin(x*y*t)')

        s = mms.moosefunction(f)
        self.assertEqual(s, '-std::pow(p(0), 2)*_r*std::pow(t, 2)*std::cos(p(0)*p(1)*t) - ' \
                            'p(0)*p(1)*std::sin(p(0)*p(1)*t) - p(0)*t*_u(1)*std::sin(p(0)*p(1)*t) ' \
                            '- std::pow(p(1), 2)*_r*std::pow(t, 2)*std::cos(p(0)*p(1)*t) - ' \
                            'p(1)*t*_u(0)*std::sin(p(0)*p(1)*t)')

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
