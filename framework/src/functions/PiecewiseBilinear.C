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

/* PiecewiseBilinear reads from a file the information necessary to build the vectors x and y and
 * the ColumnMajorMatrix z, and then sends those (along with a sample point) to BilinearInterpolation.
 * See BilinearInterpolation in moose/src/utils for a description of how that works...it is a 2D linear
 * interpolation algorithm.  The format of the data file must be the following:
 *
 * 1,2
 * 1,1,2
 * 2,3,4
 *
 * The first row is the x vector data.
 * After the first row, the first column is the y vector data.
 * The rest of the data is used to build the ColumnMajorMatrix z.
 *
 * x = [1 2]
 * y = [1 2]
 *
 * z = [1 2]
 *     [3 4]
 *
 *     z has to be x.size() by y.size()
 *
 * PiecewisBilinear also sends samples to BilinearInterpolation.  These samples are the z-coordinate of the current
 * integration point, and the current value of time.  The name of the file that contains this data has to be included
 * in the function block of the inpute file like this...yourFileName = example.csv.
 */

#include "PiecewiseBilinear.h"

template<>
InputParameters validParams<PiecewiseBilinear>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<std::string>("yourFileName", "File holding power factor data");
//  params.addParam<Real>("scale_factor", 1.0, "Scale factor to be applied to the ordinate values");
  return params;
}

PiecewiseBilinear::PiecewiseBilinear(const std::string & name, InputParameters parameters) :
  Function(name, parameters),
  _bilinear_interp( NULL ),
  _file_name( getParam<std::string>("yourFileName") )
{
  std::vector<Real> x;
  std::vector<Real> y;
  ColumnMajorMatrix z;
  
  // Parse to get x, y, z
  parse( x, y, z );
  
  _bilinear_interp = new BilinearInterpolation( x, y, z );
}

PiecewiseBilinear::~PiecewiseBilinear()
{
  delete _bilinear_interp;
}

Real
PiecewiseBilinear::value( Real t, const Point & p)
{
//  std::cout << "t " << t << std::endl;
//  std::cout << "p(2) " << p(2) << std::endl;
  
  return  _bilinear_interp->sample( p(2), t );
  
}

void
PiecewiseBilinear::parse( std::vector<Real> & x,
                          std::vector<Real> & y,
                          ColumnMajorMatrix & z)
{

  std::ifstream file(_file_name.c_str());
   std::string line;
   unsigned int linenum = 0;
   unsigned int itemnum;
   std::vector<Real> data;
     
   while (getline (file, line))
   {
     linenum++;
//     std::cout << "\nLine #" << linenum << ":" << std::endl;
     std::istringstream linestream(line);
     std::string item;
     itemnum = 0;
     while (getline (linestream, item, ','))
     {
       itemnum++;
       std::istringstream i(item);
       Real d;
       i >> d;
       data.push_back( d );
//       std::cout << "Item #" << itemnum << ": " << item << std::endl;
             
     }
   }

   x.resize(itemnum-1);
   y.resize(linenum-1);
   z.reshape(itemnum-1, linenum-1);
   unsigned int offset(0);
   // Extract the first line's data (the x axis data)
   for (unsigned int j(0); j < itemnum-1; ++j)
   {
     x[j] = data[offset];
     ++offset;
   }
   for (unsigned int i(0); i < linenum-1; ++i)
   {
     // Extract the y axis entry for this line
     y[i] = data[offset];
     ++offset;

     // Extract the function values for this row in the matrix
     for (unsigned int j(0); j < itemnum-1; ++j)
     {
       z(i,j) = data[offset];
       ++offset;
     }
   }
   if (data.size() != offset)
   {
     std::cerr << "ERROR! Data mismatch!" << std::endl;
   }
}
