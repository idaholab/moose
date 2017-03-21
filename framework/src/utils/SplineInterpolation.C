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

int SplineInterpolation::_file_number = 0;

SplineInterpolation::SplineInterpolation() {}

SplineInterpolation::SplineInterpolation(const std::vector<Real> & x,
                                         const std::vector<Real> & y,
                                         Real yp1 /* = _deriv_bound*/,
                                         Real ypn /* = _deriv_bound*/)
  : SplineInterpolationBase(), _x(x), _y(y), _yp1(yp1), _ypn(ypn)
{
  errorCheck();
  solve();
}

void
SplineInterpolation::setData(const std::vector<Real> & x,
                             const std::vector<Real> & y,
                             Real yp1 /* = _deriv_bound*/,
                             Real ypn /* = _deriv_bound*/)
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
  for (unsigned i = 0; !error && i + 1 < _x.size(); ++i)
    if (_x[i] >= _x[i + 1])
      error = true;

  if (error)
    mooseError("x-values are not strictly increasing");
}

void
SplineInterpolation::solve()
{
  spline(_x, _y, _y2, _yp1, _ypn);
}

Real
SplineInterpolation::sample(Real x) const
{
  return SplineInterpolationBase::sample(_x, _y, _y2, x);
}

Real
SplineInterpolation::sampleDerivative(Real x) const
{
  return SplineInterpolationBase::sampleDerivative(_x, _y, _y2, x);
}

Real
SplineInterpolation::sample2ndDerivative(Real x) const
{
  return SplineInterpolationBase::sample2ndDerivative(_x, _y, _y2, x);
}

Real
SplineInterpolation::domain(int i) const
{
  return _x[i];
}

Real
SplineInterpolation::range(int i) const
{
  return _y[i];
}

void
SplineInterpolation::dumpSampleFile(std::string base_name,
                                    std::string x_label,
                                    std::string y_label,
                                    float xmin,
                                    float xmax,
                                    float ymin,
                                    float ymax)
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

  for (unsigned int i = 1; i < _x.size(); ++i)
  {
    Real m = (_y[i] - _y[i - 1]) / (_x[i] - _x[i - 1]);
    Real b = (_y[i] - m * _x[i]);

    out << _x[i - 1] << "<=x && x<" << _x[i] << " ? " << m << "*x+(" << b << ") : ";
  }
  out << " 1/0\n";

  out << "\nplot f(x) with lines, '" << filename_pts.str() << "' using 1:2 title \"Points\"\n";
  out.close();

  libmesh_assert(_x.size() == _y.size());

  out.open(filename_pts.str().c_str());
  /* Next dump the data points into a seperate file */
  for (unsigned int i = 0; i < _x.size(); ++i)
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
