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
#include "TrackedObjectFactory.h"

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
  CoefficientManager(TrackedScalarCoefficientFactory & scalar_factory,
                     TrackedVectorCoefficientFactory & vector_factory,
                     TrackedMatrixCoefficientFactory & matrix_factory)
    : _scalar_factory(scalar_factory),
      _vector_factory(vector_factory),
      _matrix_factory(matrix_factory),
      _scalar_coeffs(scalar_factory),
      _vector_coeffs(vector_factory),
      _matrix_coeffs(matrix_factory)
  {
  }

  void declareScalar(const std::string & name, std::shared_ptr<mfem::Coefficient> coef);
  template <class P, class... Args>
  void declareScalar(const std::string & name, Args &&... args)
  {
    this->declareScalar(name, _scalar_factory.make<P>(args...));
  }

  void declareScalarProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             std::shared_ptr<mfem::Coefficient> coef);
  template <class P, class... Args>
  void declareScalarProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             Args &&... args)
  {
    this->declareScalarProperty(name, blocks, _scalar_factory.make<P>(args...));
  }

  void declareVector(const std::string & name, std::shared_ptr<mfem::VectorCoefficient> coef);
  template <class P, class... Args>
  void declareVector(const std::string & name, Args &&... args)
  {
    this->declareVector(name, _vector_factory.make<P>(args...));
  }

  void declareVectorProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             std::shared_ptr<mfem::VectorCoefficient> coef);
  template <class P, class... Args>
  void declareVectorProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             Args &&... args)
  {
    this->declareVectorProperty(name, blocks, _vector_factory.make<P>(args...));
  }

  void declareMatrix(const std::string & name, std::shared_ptr<mfem::MatrixCoefficient> coef);
  template <class P, class... Args>
  void declareMatrix(const std::string & name, Args &&... args)
  {
    this->declareMatrix(name, _matrix_factory.make<P>(args...));
  }

  void declareMatrixProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             std::shared_ptr<mfem::MatrixCoefficient> coef);
  template <class P, class... Args>
  void declareMatrixProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             Args &&... args)
  {
    this->declareMatrixProperty(name, blocks, _matrix_factory.make<P>(args...));
  }

  mfem::Coefficient & getScalarCoefficient(const std::string name);
  mfem::VectorCoefficient & getVectorCoefficient(const std::string name);
  mfem::MatrixCoefficient & getMatrixCoefficient(const std::string name);
  std::shared_ptr<mfem::Coefficient> getScalarCoefficientPtr(const std::string name);
  std::shared_ptr<mfem::VectorCoefficient> getVectorCoefficientPtr(const std::string name);
  std::shared_ptr<mfem::MatrixCoefficient> getMatrixCoefficientPtr(const std::string name);
  bool scalarPropertyIsDefined(const std::string & name, const std::string & block) const;
  bool vectorPropertyIsDefined(const std::string & name, const std::string & block) const;
  bool matrixPropertyIsDefined(const std::string & name, const std::string & block) const;

private:
  TrackedScalarCoefficientFactory & _scalar_factory;
  TrackedVectorCoefficientFactory & _vector_factory;
  TrackedMatrixCoefficientFactory & _matrix_factory;
  ScalarMap _scalar_coeffs;
  VectorMap _vector_coeffs;
  MatrixMap _matrix_coeffs;
};
}

#endif
