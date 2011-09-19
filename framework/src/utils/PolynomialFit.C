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

#include "PolynomialFit.h"
#include "libmesh_common.h"
#include "Moose.h"

extern "C" void dgels_ ( ... );

int PolynomialFit::_file_number = 0;

PolynomialFit::PolynomialFit(std::vector<Real> x, std::vector<Real> y, unsigned int order, bool truncate_order) :
    _x(x),
    _y(y),
    _order(order),
    _truncate_order(truncate_order)
{
  if (_truncate_order)  // && (_x.size() / 10) < _order)
  {
    if (_x.size() == 1)
      _order = 0;
    else
    {
      _order = (_x.size() / 10) + 1;

      if (_order > order)
        _order = order;
    }

  }
  else if (_x.size() < order)
    mooseError("Polynomial Fit requires an order less than the size of the input vector\n");
}

void
PolynomialFit::generate()
{
  fillMatrix();
  doLeastSquares();
}

void
PolynomialFit::fillMatrix()
{
  unsigned int num_rows = _x.size();
  unsigned int num_cols = _order+1;
  _matrix.resize(num_rows*num_cols);

  for (unsigned int col=0; col<=_order; ++col)
  {
    for (unsigned int row=0; row < num_rows; ++row)
    {
      Real value = 1;
      for (unsigned int i=0; i < col; ++i)
        value *= _x[row];

      _matrix[(col*num_rows)+row] = value;
    }
  }
}

void
PolynomialFit::doLeastSquares()
{
  char mode = 'N';
  int num_rows = _x.size();
  int num_coeff = _order + 1;
  int num_rhs = 1;
  int buffer_size = -1;
  Real opt_buffer_size;
  Real *buffer;
  int return_value = 0;

  // Must copy _y because the call to dgels destroys the original values
  std::vector<Real> rhs = _y;

  dgels_(&mode, &num_rows, &num_coeff, &num_rhs, &_matrix[0], &num_rows, &rhs[0], &num_rows, &opt_buffer_size, &buffer_size, &return_value);
  if (return_value)
    mooseError("");

  buffer_size = (int) opt_buffer_size;

  buffer = new Real[buffer_size];
  dgels_(&mode, &num_rows, &num_coeff, &num_rhs, &_matrix[0], &num_rows, &rhs[0], &num_rows, buffer, &buffer_size, &return_value);
  delete [] buffer;

  if (return_value)
    mooseError("");

  _coeffs.resize(num_coeff);
  for (int i=0; i<num_coeff; ++i)
    _coeffs[i] = rhs[i];

}

Real
PolynomialFit::sample(Real x)
{
  unsigned int size = _coeffs.size();
  Real value = 0;

  Real curr_x = 1;
  for (unsigned int i=0; i<size; ++i)
  {
    value += _coeffs[i]*curr_x;
    curr_x *= x;
  }
  return value;
}

void
PolynomialFit::dumpSampleFile(std::string base_name, std::string x_label, std::string y_label, float xmin, float xmax, float ymin, float ymax)
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

  /* First dump the GNUPLOT file with the Least Squares Equations */
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

  for (unsigned int i = 0; i<_coeffs.size(); ++i)
  {
    if (i)
      out << "+";

    out << _coeffs[i];
    for (unsigned int j = 0; j<i; ++j)
      out << "*x";
  }
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
PolynomialFit::getSampleSize()
{
  return _x.size();
}
