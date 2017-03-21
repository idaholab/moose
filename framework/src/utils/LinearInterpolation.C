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

#include "LinearInterpolation.h"

#include <stdexcept>
#include <cassert>

int LinearInterpolation::_file_number = 0;

LinearInterpolation::LinearInterpolation(const std::vector<Real> & x, const std::vector<Real> & y)
  : _x(x), _y(y)
{
  errorCheck();
}

void
LinearInterpolation::errorCheck()
{
  if (_x.size() != _y.size())
    throw std::domain_error("Vectors are not the same length");

  for (unsigned int i = 0; i + 1 < _x.size(); ++i)
    if (_x[i] >= _x[i + 1])
    {
      std::ostringstream oss;
      oss << "x-values are not strictly increasing: x[" << i << "]: " << _x[i] << " x[" << i + 1
          << "]: " << _x[i + 1];
      throw std::domain_error(oss.str());
    }
}

Real
LinearInterpolation::sample(Real x) const
{
  // sanity check (empty LinearInterpolations get constructed in many places
  // so we cannot put this into the errorCheck)
  assert(_x.size() > 0);

  // endpoint cases
  if (x <= _x[0])
    return _y[0];
  if (x >= _x.back())
    return _y.back();

  for (unsigned int i = 0; i + 1 < _x.size(); ++i)
    if (x >= _x[i] && x < _x[i + 1])
      return _y[i] + (_y[i + 1] - _y[i]) * (x - _x[i]) / (_x[i + 1] - _x[i]);

  throw std::out_of_range("Unreachable");
  return 0;
}

Real
LinearInterpolation::sampleDerivative(Real x) const
{
  // endpoint cases
  if (x < _x[0])
    return 0.0;
  if (x >= _x[_x.size() - 1])
    return 0.0;

  for (unsigned int i = 0; i + 1 < _x.size(); ++i)
    if (x >= _x[i] && x < _x[i + 1])
      return (_y[i + 1] - _y[i]) / (_x[i + 1] - _x[i]);

  throw std::out_of_range("Unreachable");
  return 0;
}

Real
LinearInterpolation::integrate()
{
  Real answer = 0;
  for (unsigned int i = 1; i < _x.size(); ++i)
    answer += 0.5 * (_y[i] + _y[i - 1]) * (_x[i] - _x[i - 1]);

  return answer;
}

Real
LinearInterpolation::domain(int i) const
{
  return _x[i];
}

Real
LinearInterpolation::range(int i) const
{
  return _y[i];
}

void
LinearInterpolation::dumpSampleFile(std::string base_name,
                                    std::string x_label,
                                    std::string y_label,
                                    Real xmin,
                                    Real xmax,
                                    Real ymin,
                                    Real ymax)
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

  assert(_x.size() == _y.size());

  out.open(filename_pts.str().c_str());
  /* Next dump the data points into a separate file */
  for (unsigned int i = 0; i < _x.size(); ++i)
    out << _x[i] << " " << _y[i] << "\n";
  out << std::endl;

  ++_file_number;
  out.close();
}

unsigned int
LinearInterpolation::getSampleSize()
{
  return _x.size();
}
