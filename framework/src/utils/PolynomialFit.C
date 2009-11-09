#include "PolynomialFit.h"
#include "libmesh_common.h"
#include "Moose.h"

extern "C" void dgels_ ( ... ); 

int PolynomialFit::_file_number = 0;

PolynomialFit::PolynomialFit(std::vector<double> x, std::vector<double> y, unsigned int order, bool truncate_order)
  :_x(x),
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
  else if (!_truncate_order) 
  {
    std::cerr << "Polynomial Fit requires an order less than the size of the input vector\n";
    mooseError("");
  }
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
      double value = 1;
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
  double opt_buffer_size;
  double *buffer; 
  int return_value = 0;

  // Must copy _y because the call to dgels destroys the original values
  std::vector<double> rhs = _y;

  dgels_(&mode, &num_rows, &num_coeff, &num_rhs, &_matrix[0], &num_rows, &rhs[0], &num_rows, &opt_buffer_size, &buffer_size, &return_value);
  if (return_value)
    mooseError("");
  
  buffer_size = (int) opt_buffer_size;
  
  buffer = new double[buffer_size];
  dgels_(&mode, &num_rows, &num_coeff, &num_rhs, &_matrix[0], &num_rows, &rhs[0], &num_rows, buffer, &buffer_size, &return_value); 
  delete [] buffer;

  if (return_value)
    mooseError("");
  
  _coeffs.resize(num_coeff);
  for (unsigned int i=0; i<num_coeff; ++i)
    _coeffs[i] = rhs[i];
  
}

double
PolynomialFit::sample(double x)
{
  unsigned int size = _coeffs.size();
  double value = 0;

  double curr_x = 1;
  for (unsigned int i=0; i<size; ++i)
  {
    value += _coeffs[i]*curr_x;
    curr_x *= x;
  }
  return value;
}

void
PolynomialFit::dumpSampleFile(unsigned int proc_id, float xmin, float xmax, float ymin, float ymax)
{
  std::stringstream filename, filename_pts;
  const unsigned char fill_character = '0';
  const unsigned int field_width = 4;

  filename.fill(fill_character);
  filename << "dump";
  filename.width(field_width);
  filename << _file_number << ".plt";

  filename_pts.fill(fill_character);
  filename_pts << "dump_pts";
  filename_pts.width(field_width);
  filename_pts << _file_number << ".dat";

  /* First dump the GNUPLOT file with the Least Squares Equations */
  std::ofstream out(filename.str().c_str());
  out.precision(15);
  out.fill(fill_character);
  
  out << "set terminal postscript color enhanced\n" 
      << "set output \"least_squares";
  out.width(field_width);
  out << _file_number << ".eps\"\n"
      << "set xlabel \"Temperature\"\n"
      << "set ylabel \"Thermal Conductivity\"\n"
      << "set xrange [" << xmin << ":" << xmax << "]\n"
      << "set yrange [" << ymin << ":" << ymax << "]\n"
      << "set key left top\n"
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

  
