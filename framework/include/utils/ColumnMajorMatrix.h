#ifndef COLUMNMAJORMATRIX_H
#define COLUMNMAJORMATRIX_H

#include "Moose.h"

// libmesh includes
#include "type_tensor.h"

// system includes
#include <vector>

/**
 * This class defines a Tensor that can change it's shape.
 *
 * This means a 3x3x3x3 Tensor can be represented as a 9x9 or an 81x1.
 *
 * Further, the values of this tensor are _COLUMN_ major ordered!
 */
class ColumnMajorMatrix
{
public:
  /**
   * Constructor that sets an initial number of entries and shape.
   *
   * Defaults to creating the same size tensor as TensorValue
   */
  ColumnMajorMatrix(const unsigned int rows = LIBMESH_DIM, const unsigned int cols = LIBMESH_DIM);

  /**
   * The total number of entries in the Tensor.
   *
   * ie cols * rows
   */
  unsigned int numEntries() const;

  /**
   * Change the shape of the tensor.
   *
   * Note that cols * rows should be equal to numEntries()!
   */
  void reshape(const unsigned int rows, const unsigned int cols);
  
  /**
   * Get the i,j entry
   *
   * j defaults to zero so you can use it as a row vector.
   */
  Real & operator()(const unsigned int i, const unsigned int j=0);

  /**
   * Get the i,j entry
   *
   * j defaults to zero so you can use it as a row vector.
   *
   * This is the version used for a const ColumnMajorMatrix.
   */
  Real operator()(const unsigned int i, const unsigned int j=0) const;

  /**
   * Print the tensor
   */
  void print();

  /**
   * Fills the passed in tensor with the values from this tensor.
   */
  void fill(TypeTensor<Real> & tensor);

  /**
   * Sets the values in _this_ tensor to the values on the RHS.
   *
   * Will also reshape this tensor if necessary.
   */
  void operator=(const TypeTensor<Real> & rhs);

  /**
   * Sets the values in _this_ tensor to the values on the RHS.
   *
   * Will also reshape this tensor if necessary.
   */
  void operator=(const ColumnMajorMatrix & rhs);

protected:
  std::vector<Real> _values;

  unsigned int _n_rows, _n_cols;
};

ColumnMajorMatrix::ColumnMajorMatrix(unsigned int rows, unsigned int cols)
  : _n_rows(rows),
    _n_cols(cols),
    _values(rows*cols)
{}

inline unsigned int
ColumnMajorMatrix::numEntries() const
{
  return _values.size();
}

inline void
ColumnMajorMatrix::reshape(unsigned int rows, unsigned int cols)
{
  mooseAssert((cols * rows) == numEntries(), "Error!  Trying to change shape to something that doesn't match the current number of entries");

  _n_rows = rows;
  _n_cols = cols;
}

inline Real &
ColumnMajorMatrix::operator()(const unsigned int i, const unsigned int j)
{
  mooseAssert((i*j) < numEntries(), "Reference outside of ColumnMajorMatrix bounds!");

  // Row major indexing!
  return _values[(j*_n_rows) + i];
}

inline Real
ColumnMajorMatrix::operator()(const unsigned int i, const unsigned int j) const
{
  mooseAssert((i*j) < numEntries(), "Reference outside of ColumnMajorMatrix bounds!");

  // Row major indexing!
  return _values[(j*_n_rows) + i];
}

inline void
ColumnMajorMatrix::print()
{
  ColumnMajorMatrix & s = (*this);
  
  for(unsigned int i=0; i<_n_rows; i++)
  {
    for(unsigned int j=0; j<_n_cols; j++)
      std::cout<<s(i,j)<<" ";

    std::cout<<std::endl;
  }
} 

inline void
ColumnMajorMatrix::fill(TypeTensor<Real> & tensor)
{
  mooseAssert(LIBMESH_DIM*LIBMESH_DIM == numEntries(), "Cannot fill tensor!  The ColumnMajorMatrix doesn't have the same number of entries!");

  ColumnMajorMatrix & s = (*this);

  for(unsigned int j=0; j<_n_cols; j++)
    for(unsigned int i=0; i<_n_cols; i++)
      tensor(i,j) = s(i,j);
}  

void
ColumnMajorMatrix::operator=(const TypeTensor<Real> & rhs)
{
  // Resize the tensor if necessary
  if((LIBMESH_DIM * LIBMESH_DIM) != numEntries())
    _values.resize(LIBMESH_DIM * LIBMESH_DIM);

  // Make sure the shape is correct
  reshape(LIBMESH_DIM, LIBMESH_DIM);

  ColumnMajorMatrix & s = (*this);

  // Copy the values
  for(unsigned int j=0; j<_n_cols; j++)
    for(unsigned int i=0; i<_n_cols; i++)
      s(i,j) = rhs(i,j);
}

void
ColumnMajorMatrix::operator=(const ColumnMajorMatrix & rhs)
{
  _n_rows = rhs._n_rows;
  _n_cols = rhs._n_cols;

  _values.resize(rhs._values.size());
  
  for(unsigned int i=0; i<_values.size(); i++)
    _values[i] = rhs._values[i];
}


#endif //COLUMNMAJORMATRIX_H
