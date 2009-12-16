#ifndef COLUMNMAJORMATRIX_H
#define COLUMNMAJORMATRIX_H

#include "Moose.h"

// libmesh includes
#include "type_tensor.h"

// system includes
#include <vector>

/**
 * This class defines a Tensor that can change it's shape.
 * This means a 3x3x3x3 Tensor can be represented as a 9x9 or an 81x1.
 * Further, the values of this tensor are _COLUMN_ major ordered!
 */
class ColumnMajorMatrix
{
public:
  /**
   * Constructor that sets an initial number of entries and shape.
   * Defaults to creating the same size tensor as TensorValue
   */
  ColumnMajorMatrix(const unsigned int rows = LIBMESH_DIM, const unsigned int cols = LIBMESH_DIM);

  /**
   * Copy Constructor defined in terms of operator=()
   */
  ColumnMajorMatrix(const ColumnMajorMatrix &rhs);

  /**
   * Constructor that fills in the ColumnMajorMatrix with values from a libMesh TypeTensor
   */
  explicit
  ColumnMajorMatrix(const TypeTensor<Real> & tensor);

  /**
   * Constructor that takes in 3 vectors and uses them to create columns
   */
  ColumnMajorMatrix(const TypeVector<Real> & col1, const TypeVector<Real> & col2, const TypeVector<Real> & col3);

  /**
   * The total number of entries in the Tensor.
   * ie cols * rows
   */
  unsigned int numEntries() const;

  /**
   * Change the shape of the tensor.
   * Note that cols * rows should be equal to numEntries()!
   */
  void reshape(const unsigned int rows, const unsigned int cols);
  
  /**
   * Get the i,j entry
   * j defaults to zero so you can use it as a column vector.
   */
  Real & operator()(const unsigned int i, const unsigned int j=0);

  /**
   * Get the i,j entry
   *
   * j defaults to zero so you can use it as a column vector.
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
   * Returns a matrix that is the transpose of the matrix this
   * was called on.
   */
  ColumnMajorMatrix transpose();

  /**
   * Sets the values in _this_ tensor to the values on the RHS.
   * Will also reshape this tensor if necessary.
   */
  ColumnMajorMatrix & operator=(const TypeTensor<Real> & rhs);

  /**
   * Sets the values in _this_ tensor to the values on the RHS
   * Will also reshape this tensor if necessary.
   */
  ColumnMajorMatrix & operator=(const ColumnMajorMatrix & rhs);

  /**
   * Scalar multiplication of the ColumnMajorMatrix
   */
  ColumnMajorMatrix operator*(Real scalar) const;

  /**
   * Matrix Vector Multiplication of the libMesh TypeVector Type
   */
  ColumnMajorMatrix operator*(const TypeVector<Real> & rhs) const;

  
//   /**
//    * Matrix Vector Multiplication of the TypeTensor Product.  Note that the
//    * Tensor type is treated as a single dimension Vector for this operation
//    */
//   ColumnMajorMatrix operator*(const TypeTensor<Real> & rhs) const;

  /**
   * Matrix Matrix Multiplication 
   */
  ColumnMajorMatrix operator*(const ColumnMajorMatrix & rhs) const;

  /**
   * Matrix Matrix Addition
   */
  ColumnMajorMatrix operator+(const ColumnMajorMatrix & rhs) const;

  /**
   * Matrix Matrix Addition plus assignment
   *
   * Note that this is faster than regular addition
   * because the result doesn't have to get copied out
   */
  ColumnMajorMatrix & operator+=(const ColumnMajorMatrix & rhs);

  /**
   * Scalar addition
   */
  ColumnMajorMatrix operator+(Real scalar) const;

  /**
   * Scalar Multiplication plus assignment
   */
  ColumnMajorMatrix & operator*=(Real scalar);

  /**
   * Scalar Addition plus assignment
   */
  ColumnMajorMatrix & operator+=(Real scalar);

  /**
   * Equality operators
   */
  bool operator==(const ColumnMajorMatrix & rhs) const;
  bool operator!=(const ColumnMajorMatrix & rhs) const;
  

protected:
  std::vector<Real> _values;

  unsigned int _n_rows, _n_cols, _n_entries;
};

inline unsigned int
ColumnMajorMatrix::numEntries() const
{
  return _n_entries;
}

inline void
ColumnMajorMatrix::reshape(unsigned int rows, unsigned int cols)
{
  mooseAssert(cols * rows == _n_entries, "Error!  Trying to change shape to something that doesn't match the current number of entries");

  _n_rows = rows;
  _n_cols = cols;
}

inline Real &
ColumnMajorMatrix::operator()(const unsigned int i, const unsigned int j)
{
  mooseAssert((i*j) < _n_entries, "Reference outside of ColumnMajorMatrix bounds!");

  // Row major indexing!
  return _values[(j*_n_rows) + i];
}

inline Real
ColumnMajorMatrix::operator()(const unsigned int i, const unsigned int j) const
{
  mooseAssert((i*j) < _n_entries, "Reference outside of ColumnMajorMatrix bounds!");

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
  mooseAssert(LIBMESH_DIM*LIBMESH_DIM == _n_entries, "Cannot fill tensor!  The ColumnMajorMatrix doesn't have the same number of entries!");

  ColumnMajorMatrix & s = (*this);

  for(unsigned int j=0; j<_n_cols; j++)
    for(unsigned int i=0; i<_n_cols; i++)
      tensor(i,j) = s(i,j);
}

inline ColumnMajorMatrix
ColumnMajorMatrix::transpose()
{
  ColumnMajorMatrix & s = (*this);

  ColumnMajorMatrix ret_matrix(_n_cols, _n_rows);

  for(unsigned int i=0; i<_n_rows; i++)
    for(unsigned int j=0; j<_n_cols; j++)
      ret_matrix(j,i) = s(i,j);

  return ret_matrix;
}



inline ColumnMajorMatrix &
ColumnMajorMatrix::operator=(const TypeTensor<Real> & rhs)
{
  // Resize the tensor if necessary
  if((LIBMESH_DIM * LIBMESH_DIM) != _n_entries)
  {
    _n_entries = LIBMESH_DIM * LIBMESH_DIM;
    _values.resize(_n_entries);
  }
  
  // Make sure the shape is correct
  reshape(LIBMESH_DIM, LIBMESH_DIM);

  ColumnMajorMatrix & s = (*this);

  // Copy the values
  for(unsigned int j=0; j<_n_cols; j++)
    for(unsigned int i=0; i<_n_cols; i++)
      s(i,j) = rhs(i,j);

  return *this;
}

inline ColumnMajorMatrix &
ColumnMajorMatrix::operator=(const ColumnMajorMatrix & rhs)
{
  _n_rows = rhs._n_rows;
  _n_cols = rhs._n_cols;
  _n_entries = rhs._n_entries;
  
  _values.resize(_n_entries);
  
  for(unsigned int i=0; i<_n_entries; i++)
    _values[i] = rhs._values[i];

  return *this;
}

inline ColumnMajorMatrix
ColumnMajorMatrix::operator*(Real scalar) const
{
  ColumnMajorMatrix ret_matrix(_n_rows, _n_cols);

  for(unsigned int i=0; i<_n_entries; i++)
    ret_matrix._values[i] = _values[i] * scalar;

  return ret_matrix;
}

inline ColumnMajorMatrix
ColumnMajorMatrix::operator*(const TypeVector<Real> & rhs) const
{
  mooseAssert(_n_cols == LIBMESH_DIM, "Cannot perform matvec operation! The column dimension of the ColumnMajorMatrix does not match the TypeVector!");

  ColumnMajorMatrix ret_matrix(_n_rows, 1);

  for (unsigned int i=0; i<_n_rows; ++i)
    for (unsigned int j=0; j<_n_cols; ++j)
      ret_matrix._values[i] += (*this)(i, j) * rhs(j);

  return ret_matrix;
}

// inline ColumnMajorMatrix
// ColumnMajorMatrix::operator*(const TypeTensor<Real> & rhs) const
// {
//   mooseAssert(_n_cols == LIBMESH_DIM*LIBMESH_DIM, "Cannot perform matvec operation!  The ColumnMajorMatrix doesn't have the correct shape!");

//   ColumnMajorMatrix ret_matrix(_n_rows, 1);

//   for (unsigned int i=0; i<_n_rows; ++i)
//     for (unsigned int j=0; j<_n_cols; ++j)
//       // Treat the Tensor as a column major column vector
//       ret_matrix._values[i] += (*this)(i, j) * rhs(j%3, j/3);
  
//   return ret_matrix;
// }

inline ColumnMajorMatrix
ColumnMajorMatrix::operator*(const ColumnMajorMatrix & rhs) const
{
  mooseAssert(_n_cols == rhs._n_rows, "Cannot perform matrix multiply!  The shapes of the two operands are not compatible!");

  ColumnMajorMatrix ret_matrix(_n_rows, rhs._n_cols);

  for (unsigned int i=0; i<ret_matrix._n_rows; ++i)
    for (unsigned int j=0; j<ret_matrix._n_cols; ++j) 
      for (unsigned int k=0; k<_n_cols; ++k)
        ret_matrix(i, j) += (*this)(i, k) * rhs(k, j);
  
  return ret_matrix;
}

inline ColumnMajorMatrix
ColumnMajorMatrix::operator+(const ColumnMajorMatrix & rhs) const
{
  mooseAssert((_n_rows == rhs._n_rows) && (_n_cols == rhs._n_cols), "Cannot perform matrix addition!  The shapes of the two operands are not compatible!");

  ColumnMajorMatrix ret_matrix(_n_rows, _n_cols);

  for(unsigned int i=0; i<_n_entries; i++)
    ret_matrix._values[i] = _values[i] + rhs._values[i];

  return ret_matrix;
}

inline ColumnMajorMatrix &
ColumnMajorMatrix::operator+=(const ColumnMajorMatrix & rhs)
{
  mooseAssert((_n_rows == rhs._n_rows) && (_n_cols == rhs._n_cols), "Cannot perform matrix addition and assignment!  The shapes of the two operands are not compatible!");

  for(unsigned int i=0; i<_n_entries; i++)
    _values[i] += rhs._values[i];

  return *this;
}

inline ColumnMajorMatrix
ColumnMajorMatrix::operator+(Real scalar) const
{
  ColumnMajorMatrix ret_matrix(_n_rows, _n_cols);
  
  for(unsigned int i=0; i<_n_entries; i++)
    ret_matrix._values[i] = _values[i] + scalar;
  
  return ret_matrix;
}

inline ColumnMajorMatrix &
ColumnMajorMatrix::operator*=(Real scalar)
{
  for(unsigned int i=0; i<_n_entries; i++)
    _values[i] *= scalar;
  return *this;
}

inline ColumnMajorMatrix &
ColumnMajorMatrix::operator+=(Real scalar)
{
  for(unsigned int i=0; i<_n_entries; i++)
    _values[i] += scalar;
  return *this;
}

inline bool
ColumnMajorMatrix::operator==(const ColumnMajorMatrix & rhs) const
{
  if (_n_entries != rhs._n_entries || _n_rows != rhs._n_rows || _n_cols != rhs._n_cols)
    return false;
  return std::equal(_values.begin(), _values.end(), rhs._values.begin());
}

inline bool
ColumnMajorMatrix::operator!=(const ColumnMajorMatrix & rhs) const
{
  return !(*this == rhs);
}


#endif //COLUMNMAJORMATRIX_H
