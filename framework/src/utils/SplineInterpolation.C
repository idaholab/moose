/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SplineInterpolation.h"
#include "MooseError.h"
#include "libmesh/libmesh_common.h"

int SplineInterpolation::_file_number = 0;

SplineInterpolation::SplineInterpolation()
{
}

SplineInterpolation::SplineInterpolation(const std::vector<double> & x, const std::vector<double> & y, double yp1/* = 1e30*/, double ypn/* = 1e30*/) :
    _x(x),
    _y(y),
    _yp1(yp1),
    _ypn(ypn)
{
  errorCheck();
  solve();
}

void
SplineInterpolation::setData(const std::vector<double> & x, const std::vector<double> & y, double yp1/* = 1e30*/, double ypn/* = 1e30*/)
{
  _x = x;
  _y = y;
  _yp1 = yp1;
  _ypn = ypn;
  errorCheck();
  solve();
}

void
SplineInterpolation::errorCheck()
{
  if (_x.size() != _y.size())
    mooseError("SplineInterpolation: vectors are not the same length");

  bool error = false;
  for (unsigned i = 0; !error && !_x.empty() && i < _x.size() - 1; ++i)
  {
    if (_x[i] >= _x[i+1])
      error = true;
  }

  if (error)
    mooseError( "x-values are not strictly increasing" );
}

void
SplineInterpolation::solve()
{
  unsigned int n = _x.size();
  std::vector<double> u(n, 0.);
  _y2.resize(n, 0.);

  if (_yp1 >= 1e30)
    _y2[0] = u[0] = 0.;
  else
  {
    _y2[0] = -0.5;
    u[0] = (3.0 / (_x[1] - _x[0])) * ((_y[1] - _y[0]) / (_x[1] - _x[0]) - _yp1);
  }
  // decomposition of tri-diagonal algorithm (_y2 and u are used for temporary storage)
  for (unsigned int i = 1; i < n-1; i++)
  {
    double sig = (_x[i] - _x[i - 1]) / (_x[i + 1] - _x[i - 1]);
    double p = sig * _y2[i - 1] + 2.0;
    _y2[i] = (sig - 1.0) / p;
    u[i] = (_y[i + 1] - _y[i]) / (_x[i + 1] - _x[i]) - (_y[i] - _y[i - 1]) / (_x[i] - _x[i - 1]);
    u[i] = (6.0 * u[i] / (_x[i+1] - _x[i - 1]) - sig * u[i - 1]) / p;
  }

  double qn, un;
  if (_ypn >= 1e30)
    qn = un = 0.;
  else
  {
    qn = 0.5;
    un = (3.0 / (_x[n - 1] - _x[n - 2])) * (_ypn - (_y[n - 1] - _y[n - 2]) / (_x[n - 1] - _x[n - 2]));
  }

  _y2[n - 1] = (un - qn * u[n - 2]) / (qn * _y2[n - 2] + 1.);
  // back substitution
  for (int k = n - 2; k >= 0; k--)
    _y2[k] = _y2[k] * _y2[k + 1] + u[k];
}

void
SplineInterpolation::findInterval(double x, unsigned int & klo, unsigned int & khi) const
{
  klo = 0;
  khi = _x.size() - 1;
  while (khi - klo > 1)
  {
    unsigned int k = (khi + klo) >> 1;
    if (_x[k] > x)
      khi = k;
    else
      klo = k;
  }
}

void
SplineInterpolation::computeCoeffs(unsigned int klo, unsigned int khi, double x, double & h, double & a, double & b) const
{
  h = _x[khi] - _x[klo];
  if (h == 0)
    mooseError("The values of x must be distinct");
  a = (_x[khi] - x) / h;
  b = (x - _x[klo]) / h;
}

double
SplineInterpolation::sample(double x) const
{
  unsigned int klo, khi;
  findInterval(x, klo, khi);

  double h, a, b;
  computeCoeffs(klo, khi, x, h, a, b);

  return a * _y[klo] + b * _y[khi] + ((a*a*a - a) * _y2[klo] + (b*b*b - b) * _y2[khi]) * (h*h) / 6.0;
}

double
SplineInterpolation::sampleDerivative(double x) const
{
  unsigned int klo, khi;
  findInterval(x, klo, khi);

  double h, a, b;
  computeCoeffs(klo, khi, x, h, a, b);

  return (_y[khi] - _y[klo]) / h - (((3. * a*a - 1) * _y2[klo] + (3. * b*b - 1.) * _y2[khi]) * h / 6);

}

double
SplineInterpolation::sample2ndDerivative(double x) const
{
  unsigned int klo, khi;
  findInterval(x, klo, khi);

  double h, a, b;
  computeCoeffs(klo, khi, x, h, a, b);

  return a * _y2[klo] + b * _y2[khi];
}

double
SplineInterpolation::domain(int i) const
{
  return _x[i];
}

double
SplineInterpolation::range(int i) const
{
  return _y[i];
}

void
SplineInterpolation::dumpSampleFile(std::string base_name, std::string x_label, std::string y_label, float xmin, float xmax, float ymin, float ymax)
{
  std::stringstream filename, filename_pts;
  const unsigned char fill_character = '0';
  const unsigned int field_width = 4;

  filename.fill(fill_character);
  filename << base_name;
  filename.width(field_width);
  filename << _file_number << ".plt";

  filename_pts.fill(fill_character);
  filename_pts << base_name << "_pts";
  filename_pts.width(field_width);
  filename_pts << _file_number << ".dat";

  /* First dump the GNUPLOT file with the Piecewise Linear Equations */
  std::ofstream out(filename.str().c_str());
  out.precision(15);
  out.fill(fill_character);

  out << "set terminal postscript color enhanced\n"
      << "set output \"" << base_name;
  out.width(field_width);
  out << _file_number << ".eps\"\n"
      << "set xlabel \"" << x_label << "\"\n"
      << "set ylabel \"" << y_label << "\"\n";
  if (xmin != 0 && xmax != 0)
    out << "set xrange [" << xmin << ":" << xmax << "]\n";
  if (ymin != 0 && ymax != 0)
    out << "set yrange [" << ymin << ":" << ymax << "]\n";
  out << "set key left top\n"
      << "f(x)=";

   for (unsigned int i=1; i<_x.size(); ++i)
   {
     Real m = (_y[i] - _y[i-1])/(_x[i] - _x[i-1]);
     Real b = (_y[i] - m*_x[i]);

     out << _x[i-1] << "<=x && x<" << _x[i] << " ? " << m << "*x+(" << b << ") : ";
   }
   out << " 1/0\n";

  out << "\nplot f(x) with lines, '" << filename_pts.str() << "' using 1:2 title \"Points\"\n";
  out.close();

  libmesh_assert(_x.size() == _y.size());

  out.open(filename_pts.str().c_str());
  /* Next dump the data points into a seperate file */
  for (unsigned int i = 0; i<_x.size(); ++i)
    out << _x[i] << " " << _y[i] << "\n";
  out << std::endl;

  ++_file_number;
  out.close();
}

unsigned int
SplineInterpolation::getSampleSize()
{
  return _x.size();
}
