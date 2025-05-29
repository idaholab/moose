//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#pragma once
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "MooseException.h"

#include "libmesh/ignore_warnings.h"
#include <mfem.hpp>
#include "libmesh/restore_warnings.h"
#include "CoefficientMap.h"

namespace Moose::MFEM
{
/**
 * Front-end class for creating and storing MFEM coefficients. They
 * can be created so they are global (defined across the entire
 * domain) or as piecewise material properties.
 */
class CoefficientManager
{
public:
  CoefficientManager() = default;

  /// Declare an alias to an existing scalar coefficient or, if it
  /// does not exist, try interpreting the name as a number with which
  /// to create a new constant coefficient.
  mfem::Coefficient & declareScalar(const std::string & name, const std::string & existing_or_literal);
  /// Create a new scalar coefficient, constructed from the argument pack
  template <class P, class... Args>
  P & declareScalar(const std::string & name, Args &&... args)
  {
    auto coef = _scalar_coeffs.make<P>(args...);
    this->declareScalar(name, coef);
    return *coef;
  }

  /**
   * Use an existing scalar coefficient for a property on some blocks
   * of the mesh. The property will be a piecewise coefficient and it
   * will have the value of `existing_coef` on these blocks. If no
   * such scalar coefficient exists, try interpreting the name as a
   * number with which to create a new constant coefficient.
   */
  mfem::Coefficient & declareScalarProperty(const std::string & name,
                                            const std::vector<std::string> & blocks,
                                            const std::string & existing_or_literal);
  /**
   * Use a new scalar coefficient, constructed from the argument pack, for a
   * property on some blocks of the mesh. The property will be a piecewise
   * coefficient and it will have the value of the new coefficient on these
   * blocks.
   */
  template <class P, class... Args>
  mfem::Coefficient & declareScalarProperty(const std::string & name,
                                            const std::vector<std::string> & blocks,
                                            Args &&... args)
  {
    return this->declareScalarProperty(name, blocks, _scalar_coeffs.make<P>(args...));
  }

  /// Declare an alias to an existing vector coefficientor or, if it
  /// does not exist, try interpreting the name as a vector of numbers with which
  /// to create a new constant vector coefficient.
  mfem::VectorCoefficient & declareVector(const std::string & name,
                                          const std::string & existing_or_literal);
  /// Create a new vector coefficient, constructed from the argument pack.
  template <class P, class... Args>
  P & declareVector(const std::string & name, Args &&... args)
  {
    auto coef = _vector_coeffs.make<P>(args...);
    this->declareVector(name, coef);
    return *coef;
  }

  /**
   * Use an existing vector coefficient for a property on some blocks
   * of the mesh. The property will be a piecewise coefficient and it
   * will have the value of `existing_coef` on these blocks. If no
   * such vector coefficient exists, try interpreting the name as a
   * vector of numbers with which to create a new constant vector
   * coefficient.
   */
  mfem::VectorCoefficient & declareVectorProperty(const std::string & name,
                                                  const std::vector<std::string> & blocks,
                                                  const std::string & existing_or_literal);
  /**
   * Use a new vector coefficient, constructed from the argument pack, for a
   * property on some blocks of the mesh. The property will be a piecewise
   * coefficient and it will have the value of the new coefficient on these
   * blocks.
   */
  template <class P, class... Args>
  mfem::VectorCoefficient & declareVectorProperty(const std::string & name,
                                                  const std::vector<std::string> & blocks,
                                                  Args &&... args)
  {
    return this->declareVectorProperty(name, blocks, _vector_coeffs.make<P>(args...));
  }

  /// Declare an alias to an existing matrix coefficient. Unlike for
  /// the scalar and vector counterparts, there is currently no way to
  /// try interpreting the name as numbers with which to construct a
  /// constant matrix coefficient.
  mfem::MatrixCoefficient & declareMatrix(const std::string & name,
                                          const std::string & existing_coef);
  /// Create a new matrix coefficient, constructed from the argument pack.
  template <class P, class... Args>
  P & declareMatrix(const std::string & name, Args &&... args)
  {
    auto coef = _matrix_coeffs.make<P>(args...);
    this->declareMatrix(name, coef);
    return *coef;
  }

  /**
   * Use an existing matrix coefficient for a property on some blocks of the
   * mesh. The property will be a piecewise coefficient and it will have
   * the value of `existing_coef` on these blocks. Unlike for
   * the scalar and vector counterparts, there is currently no way to
   * try interpreting the name as numbers with which to construct a
   * constant matrix coefficient.
   */
  mfem::MatrixCoefficient & declareMatrixProperty(const std::string & name,
                                                  const std::vector<std::string> & blocks,
                                                  const std::string & existing_coef);
  /**
   * Use a new matrix coefficient, constructed from the argument pack, for a
   * property on some blocks of the mesh. The property will be a piecewise
   * coefficient and it will have the value of the new coefficient on these
   * blocks.
   */
  template <class P, class... Args>
  mfem::MatrixCoefficient & declareMatrixProperty(const std::string & name,
                                                  const std::vector<std::string> & blocks,
                                                  Args &&... args)
  {
    return this->declareMatrixProperty(name, blocks, _matrix_coeffs.make<P>(args...));
  }

  /// Return a scalar coefficient with the given name or, if that
  /// doesn't exists, try interpreting the name as a number with which
  /// to build a new constant coefficient.
  mfem::Coefficient & getScalarCoefficient(const std::string name);

  /// Return a vector coefficient with the given name or, if that
  /// doesn't exists, try interpreting the name as a vector of number with which
  /// to build a new constant vector coefficient.
  mfem::VectorCoefficient & getVectorCoefficient(const std::string name);

  /// Return scalar coefficient with the given name. Unlike for
  /// the scalar and vector counterparts, there is currently no way to
  /// try interpreting the name as numbers with which to construct a
  /// constant matrix coefficient.
  mfem::MatrixCoefficient & getMatrixCoefficient(const std::string name);
  
  bool scalarPropertyIsDefined(const std::string & name, const std::string & block) const;
  bool vectorPropertyIsDefined(const std::string & name, const std::string & block) const;
  bool matrixPropertyIsDefined(const std::string & name, const std::string & block) const;
  void setTime(const double time);

private:
  ScalarMap _scalar_coeffs;
  VectorMap _vector_coeffs;
  MatrixMap _matrix_coeffs;

  mfem::Coefficient & declareScalar(const std::string & name,
                                    std::shared_ptr<mfem::Coefficient> coef);
  mfem::Coefficient & declareScalarProperty(const std::string & name,
                                            const std::vector<std::string> & blocks,
                                            std::shared_ptr<mfem::Coefficient> coef);
  mfem::VectorCoefficient & declareVector(const std::string & name,
                                          std::shared_ptr<mfem::VectorCoefficient> coef);
  mfem::VectorCoefficient & declareVectorProperty(const std::string & name,
                                                  const std::vector<std::string> & blocks,
                                                  std::shared_ptr<mfem::VectorCoefficient> coef);
  mfem::MatrixCoefficient & declareMatrix(const std::string & name,
                                          std::shared_ptr<mfem::MatrixCoefficient> coef);
  mfem::MatrixCoefficient & declareMatrixProperty(const std::string & name,
                                                  const std::vector<std::string> & blocks,
                                                  std::shared_ptr<mfem::MatrixCoefficient> coef);
  std::shared_ptr<mfem::Coefficient> getScalarCoefficientPtr(const std::string name);
  std::shared_ptr<mfem::VectorCoefficient> getVectorCoefficientPtr(const std::string name);
  std::shared_ptr<mfem::MatrixCoefficient> getMatrixCoefficientPtr(const std::string name);
};
}

#endif
