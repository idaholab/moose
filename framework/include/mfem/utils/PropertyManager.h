#pragma once
#include <map>
#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "MooseException.h"

#include "mfem.hpp"
#include "property_map.h"

namespace platypus
{

/**
 * Class to manage material properties and associate them with MFEM
 * coefficients. In particular, it will handle the complexity of
 * piecewise coefficients being built up from multiple materials.
 */
class PropertyManager
{
public:
  PropertyManager(ScalarCoefficientManager & scalar_manager,
                  VectorCoefficientManager & vector_manager,
                  MatrixCoefficientManager & matrix_manager)
    : _scalar_manager(scalar_manager),
      _vector_manager(vector_manager),
      _matrix_manager(matrix_manager),
      _scalar_coeffs(scalar_manager),
      _vector_coeffs(vector_manager),
      _matrix_coeffs(matrix_manager)
  {
  }

  void declareScalar(const std::string & name,
                     mfem::real_t value,
                     const std::vector<std::string> & blocks = {});
  void declareScalar(const std::string & name,
                     std::function<mfem::real_t(const mfem::Vector &)> func,
                     const std::vector<std::string> & blocks = {});
  void declareScalar(const std::string & name,
                     std::function<mfem::real_t(const mfem::Vector &, mfem::real_t)> func,
                     const std::vector<std::string> & blocks = {});
  void declareScalar(const std::string & name,
                     std::shared_ptr<mfem::Coefficient> coef,
                     const std::vector<std::string> & blocks = {});
  // New factory methods?
  template <class P, class... Args>
  void
  declareScalar(const std::string & name, const std::vector<std::string> & blocks, Args &&... args)
  {
    this->declareScalar(name, _scalar_manager.make<P>(args...), blocks);
  }
  template <class P, class... Args>
  void declareGlobalScalar(const std::string & name, Args &&... args)
  {
    this->declareScalar<P>(name, {}, args...);
  }

  void declareVector(const std::string & name,
                     const mfem::Vector & value,
                     const std::vector<std::string> & blocks = {});
  void declareVector(const std::string & name,
                     int dim,
                     std::function<void(const mfem::Vector &, mfem::Vector &)> func,
                     const std::vector<std::string> & blocks = {});
  void declareVector(const std::string & name,
                     int dim,
                     std::function<void(const mfem::Vector &, double t, mfem::Vector &)> func,
                     const std::vector<std::string> & blocks = {});
  void declareVector(const std::string & name,
                     std::shared_ptr<mfem::VectorCoefficient> coef,
                     const std::vector<std::string> & blocks = {});

  void declareMatrix(const std::string & name,
                     const mfem::DenseMatrix & value,
                     const std::vector<std::string> & blocks = {});
  void declareMatrix(const std::string & name,
                     int dim,
                     std::function<void(const mfem::Vector &, mfem::DenseMatrix &)> func,
                     const std::vector<std::string> & blocks = {});
  void
  declareMatrix(const std::string & name,
                int dim,
                std::function<void(const mfem::Vector &, mfem::real_t, mfem::DenseMatrix &)> func,
                const std::vector<std::string> & blocks = {});
  void declareMatrix(const std::string & name,
                     std::shared_ptr<mfem::MatrixCoefficient> coef,
                     const std::vector<std::string> & blocks = {});

  mfem::Coefficient & getScalarProperty(const std::string name);
  mfem::VectorCoefficient & getVectorProperty(const std::string name);
  mfem::MatrixCoefficient & getMatrixProperty(const std::string name);
  bool scalarIsDefined(const std::string & name, const std::string & block) const;
  bool vectorIsDefined(const std::string & name, const std::string & block) const;
  bool matrixIsDefined(const std::string & name, const std::string & block) const;

private:
  ScalarCoefficientManager & _scalar_manager;
  VectorCoefficientManager & _vector_manager;
  MatrixCoefficientManager & _matrix_manager;
  ScalarMap _scalar_coeffs;
  VectorMap _vector_coeffs;
  MatrixMap _matrix_coeffs;
};
}
