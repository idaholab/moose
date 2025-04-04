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
#include "coefficient_map.h"

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

  /// Declare an alias to an existing coefficient
  mfem::Coefficient & declareScalar(const std::string & name, const std::string & existing_coef);
  template <class P, class... Args>
  P & declareScalar(const std::string & name, Args &&... args)
  {
    auto coef = _scalar_coeffs.make<P>(args...);
    this->declareScalar(name, coef);
    return *coef;
  }

  /// Use an existing coefficient for a property on some blocks of the mesh
  mfem::Coefficient & declareScalarProperty(const std::string & name,
                                            const std::vector<std::string> & blocks,
                                            const std::string & existing_coef);
  template <class P, class... Args>
  mfem::Coefficient & declareScalarProperty(const std::string & name,
                                            const std::vector<std::string> & blocks,
                                            Args &&... args)
  {
    return this->declareScalarProperty(name, blocks, _scalar_coeffs.make<P>(args...));
  }

  /// Declare an alias to an existing coefficient
  mfem::VectorCoefficient & declareVector(const std::string & name,
                                          const std::string & existing_coef);
  template <class P, class... Args>
  P & declareVector(const std::string & name, Args &&... args)
  {
    auto coef = _vector_coeffs.make<P>(args...);
    this->declareVector(name, coef);
    return *coef;
  }

  /// Use an existing coefficient for a property on some blocks of the mesh
  mfem::VectorCoefficient & declareVectorProperty(const std::string & name,
                                                  const std::vector<std::string> & blocks,
                                                  const std::string & existing_coef);
  template <class P, class... Args>
  mfem::VectorCoefficient & declareVectorProperty(const std::string & name,
                                                  const std::vector<std::string> & blocks,
                                                  Args &&... args)
  {
    return this->declareVectorProperty(name, blocks, _vector_coeffs.make<P>(args...));
  }

  /// Declare an alias to an existing coefficient
  mfem::MatrixCoefficient & declareMatrix(const std::string & name,
                                          const std::string & existing_coef);

  template <class P, class... Args>
  P & declareMatrix(const std::string & name, Args &&... args)
  {
    auto coef = _matrix_coeffs.make<P>(args...);
    this->declareMatrix(name, coef);
    return *coef;
  }

  /// Use an existing coefficient for a property on some blocks of the mesh
  mfem::MatrixCoefficient & declareMatrixProperty(const std::string & name,
                                                  const std::vector<std::string> & blocks,
                                                  const std::string & existing_coef);
  template <class P, class... Args>
  mfem::MatrixCoefficient & declareMatrixProperty(const std::string & name,
                                                  const std::vector<std::string> & blocks,
                                                  Args &&... args)
  {
    return this->declareMatrixProperty(name, blocks, _matrix_coeffs.make<P>(args...));
  }

  mfem::Coefficient & getScalarCoefficient(const std::string name);
  mfem::VectorCoefficient & getVectorCoefficient(const std::string name);
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
};
}

#endif
