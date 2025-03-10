#ifdef MFEM_ENABLED

#include "PropertyManager.h"
#include "MooseError.h"
#include <algorithm>

namespace MooseMFEM
{
;

template <class T, class Tpw>
inline void
declareCoefficient(PropertyMap<T, Tpw> & map,
                   const std::string & name,
                   const std::vector<std::string> & blocks,
                   std::shared_ptr<T> coef,
                   const ObjectManager<T> & libmesh_dbg_var(manager))
{
  mooseAssert(std::find(manager.cbegin(), manager.cend(), coef) != manager.cend(),
              "Coefficient object was not created by the appropriate coefficient manager.");
  if (blocks.empty())
  {
    map.addProperty(name, coef);
  }
  else
  {
    map.addPiecewiseBlocks(name, coef, blocks);
  }
}

void
PropertyManager::declareScalar(const std::string & name, std::shared_ptr<mfem::Coefficient> coef)
{
  this->declareScalar(name, std::vector<std::string>(), coef);
}

void
PropertyManager::declareScalar(const std::string & name,
                               PropertyManager::global_t,
                               std::shared_ptr<mfem::Coefficient> coef)
{
  this->declareScalar(name, std::vector<std::string>(), coef);
}

void
PropertyManager::declareScalar(const std::string & name,
                               const std::vector<std::string> & blocks,
                               std::shared_ptr<mfem::Coefficient> coef)
{
  declareCoefficient(this->_scalar_coeffs, name, blocks, coef, this->_scalar_manager);
}

void
PropertyManager::declareVector(const std::string & name,
                               std::shared_ptr<mfem::VectorCoefficient> coef)
{
  this->declareVector(name, std::vector<std::string>(), coef);
}

void
PropertyManager::declareVector(const std::string & name,
                               PropertyManager::global_t,
                               std::shared_ptr<mfem::VectorCoefficient> coef)
{
  this->declareVector(name, std::vector<std::string>(), coef);
}

void
PropertyManager::declareVector(const std::string & name,
                               const std::vector<std::string> & blocks,
                               std::shared_ptr<mfem::VectorCoefficient> coef)
{
  declareCoefficient(this->_vector_coeffs, name, blocks, coef, this->_vector_manager);
}

void
PropertyManager::declareMatrix(const std::string & name,
                               std::shared_ptr<mfem::MatrixCoefficient> coef)
{
  this->declareMatrix(name, std::vector<std::string>(), coef);
}

void
PropertyManager::declareMatrix(const std::string & name,
                               PropertyManager::global_t,
                               std::shared_ptr<mfem::MatrixCoefficient> coef)
{
  this->declareMatrix(name, std::vector<std::string>(), coef);
}

void
PropertyManager::declareMatrix(const std::string & name,
                               const std::vector<std::string> & blocks,
                               std::shared_ptr<mfem::MatrixCoefficient> coef)
{
  declareCoefficient(this->_matrix_coeffs, name, blocks, coef, this->_matrix_manager);
}

mfem::Coefficient &
PropertyManager::getScalarProperty(const std::string name)
{
  return this->_scalar_coeffs.getCoefficient(name);
}

mfem::VectorCoefficient &
PropertyManager::getVectorProperty(const std::string name)
{
  return this->_vector_coeffs.getCoefficient(name);
}

mfem::MatrixCoefficient &
PropertyManager::getMatrixProperty(const std::string name)
{
  return this->_matrix_coeffs.getCoefficient(name);
}

bool
PropertyManager::scalarIsDefined(const std::string & name, const std::string & block) const
{
  return this->_scalar_coeffs.coefficientDefinedOnBlock(name, block);
}

bool
PropertyManager::vectorIsDefined(const std::string & name, const std::string & block) const
{
  return this->_vector_coeffs.coefficientDefinedOnBlock(name, block);
}

bool
PropertyManager::matrixIsDefined(const std::string & name, const std::string & block) const
{
  return this->_matrix_coeffs.coefficientDefinedOnBlock(name, block);
}
}

#endif
