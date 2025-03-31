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
#include "PropertyMap.h"
#include "ObjectManager.h"

namespace Moose::MFEM
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

  struct global_t
  {
  };

  void declareScalar(const std::string & name, std::shared_ptr<mfem::Coefficient> coef);
  template <class P, class... Args>
  void declareScalar(const std::string & name, Args &&... args)
  {
    this->declareScalarProperty<P>(name, std::vector<std::string>(), args...);
  }

  void declareScalarProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             std::shared_ptr<mfem::Coefficient> coef);
  template <class P, class... Args>
  void declareScalarProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             Args &&... args)
  {
    this->declareScalarProperty(name, blocks, _scalar_manager.make<P>(args...));
  }

  void declareVector(const std::string & name, std::shared_ptr<mfem::VectorCoefficient> coef);
  template <class P, class... Args>
  void declareVector(const std::string & name, Args &&... args)
  {
    this->declareVectorProperty<P>(name, std::vector<std::string>(), args...);
  }

  void declareVectorProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             std::shared_ptr<mfem::VectorCoefficient> coef);
  template <class P, class... Args>
  void declareVectorProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             Args &&... args)
  {
    this->declareVectorProperty(name, blocks, _vector_manager.make<P>(args...));
  }

  void declareMatrix(const std::string & name, std::shared_ptr<mfem::MatrixCoefficient> coef);
  template <class P, class... Args>
  void declareMatrix(const std::string & name, Args &&... args)
  {
    this->declareMatrixProperty<P>(name, std::vector<std::string>(), args...);
  }

  void declareMatrixProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             std::shared_ptr<mfem::MatrixCoefficient> coef);
  template <class P, class... Args>
  void declareMatrixProperty(const std::string & name,
                             const std::vector<std::string> & blocks,
                             Args &&... args)
  {
    this->declareMatrixProperty(name, blocks, _matrix_manager.make<P>(args...));
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

  const static global_t global;

private:
  ScalarCoefficientManager & _scalar_manager;
  VectorCoefficientManager & _vector_manager;
  MatrixCoefficientManager & _matrix_manager;
  ScalarMap _scalar_coeffs;
  VectorMap _vector_coeffs;
  MatrixMap _matrix_coeffs;
};
}

#endif
