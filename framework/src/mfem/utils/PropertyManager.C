#ifdef MFEM_ENABLED

#include "PropertyManager.h"
#include "MooseError.h"
#include <algorithm>

namespace Moose::MFEM
{
template <class T, class Tpw>
inline void
declareCoefficient(PropertyMap<T, Tpw> & map,
                   const std::string & name,
                   const std::vector<std::string> & blocks,
                   std::shared_ptr<T> coef,
                   const ObjectManager<T> & libmesh_dbg_var(manager))
{
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
  this->declareScalarProperty(name, std::vector<std::string>(), coef);
}

void
PropertyManager::declareScalarProperty(const std::string & name,
                                       const std::vector<std::string> & blocks,
                                       std::shared_ptr<mfem::Coefficient> coef)
{
  declareCoefficient(this->_scalar_coeffs, name, blocks, coef, this->_scalar_manager);
}

void
PropertyManager::declareVector(const std::string & name,
                               std::shared_ptr<mfem::VectorCoefficient> coef)
{
  this->declareVectorProperty(name, std::vector<std::string>(), coef);
}

void
PropertyManager::declareVectorProperty(const std::string & name,
                                       const std::vector<std::string> & blocks,
                                       std::shared_ptr<mfem::VectorCoefficient> coef)
{
  declareCoefficient(this->_vector_coeffs, name, blocks, coef, this->_vector_manager);
}

void
PropertyManager::declareMatrix(const std::string & name,
                               std::shared_ptr<mfem::MatrixCoefficient> coef)
{
  this->declareMatrixProperty(name, std::vector<std::string>(), coef);
}

void
PropertyManager::declareMatrixProperty(const std::string & name,
                                       const std::vector<std::string> & blocks,
                                       std::shared_ptr<mfem::MatrixCoefficient> coef)
{
  declareCoefficient(this->_matrix_coeffs, name, blocks, coef, this->_matrix_manager);
}

mfem::Coefficient &
PropertyManager::getScalarCoefficient(const std::string name)
{
  return this->_scalar_coeffs.getCoefficient(name);
}

mfem::VectorCoefficient &
PropertyManager::getVectorCoefficient(const std::string name)
{
  return this->_vector_coeffs.getCoefficient(name);
}

mfem::MatrixCoefficient &
PropertyManager::getMatrixCoefficient(const std::string name)
{
  return this->_matrix_coeffs.getCoefficient(name);
}

std::shared_ptr<mfem::Coefficient>
PropertyManager::getScalarCoefficientPtr(const std::string name)
{
  return this->_scalar_coeffs.getCoefficientPtr(name);
}

std::shared_ptr<mfem::VectorCoefficient>
PropertyManager::getVectorCoefficientPtr(const std::string name)
{
  return this->_vector_coeffs.getCoefficientPtr(name);
}

std::shared_ptr<mfem::MatrixCoefficient>
PropertyManager::getMatrixCoefficientPtr(const std::string name)
{
  return this->_matrix_coeffs.getCoefficientPtr(name);
}

bool
PropertyManager::scalarPropertyIsDefined(const std::string & name, const std::string & block) const
{
  return this->_scalar_coeffs.coefficientDefinedOnBlock(name, block);
}

bool
PropertyManager::vectorPropertyIsDefined(const std::string & name, const std::string & block) const
{
  return this->_vector_coeffs.coefficientDefinedOnBlock(name, block);
}

bool
PropertyManager::matrixPropertyIsDefined(const std::string & name, const std::string & block) const
{
  return this->_matrix_coeffs.coefficientDefinedOnBlock(name, block);
}
}

#endif
