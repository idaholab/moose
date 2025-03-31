#include "CoefficientManager.h"
#include <algorithm>

namespace platypus
{

void
CoefficientManager::declareScalar(const std::string & name, std::shared_ptr<mfem::Coefficient> coef)
{
  this->_scalar_coeffs.addCoefficient(name, coef);
}

void
CoefficientManager::declareScalarProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          std::shared_ptr<mfem::Coefficient> coef)
{
  this->_scalar_coeffs.addPiecewiseBlocks(name, coef, blocks);
}

void
CoefficientManager::declareVector(const std::string & name,
                                  std::shared_ptr<mfem::VectorCoefficient> coef)
{
  this->_vector_coeffs.addCoefficient(name, coef);
}

void
CoefficientManager::declareVectorProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          std::shared_ptr<mfem::VectorCoefficient> coef)
{
  this->_vector_coeffs.addPiecewiseBlocks(name, coef, blocks);
}

void
CoefficientManager::declareMatrix(const std::string & name,
                                  std::shared_ptr<mfem::MatrixCoefficient> coef)
{
  this->_matrix_coeffs.addCoefficient(name, coef);
}

void
CoefficientManager::declareMatrixProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          std::shared_ptr<mfem::MatrixCoefficient> coef)
{
  this->_matrix_coeffs.addPiecewiseBlocks(name, coef, blocks);
}

mfem::Coefficient &
CoefficientManager::getScalarCoefficient(const std::string name)
{
  return this->_scalar_coeffs.getCoefficient(name);
}

mfem::VectorCoefficient &
CoefficientManager::getVectorCoefficient(const std::string name)
{
  return this->_vector_coeffs.getCoefficient(name);
}

mfem::MatrixCoefficient &
CoefficientManager::getMatrixCoefficient(const std::string name)
{
  return this->_matrix_coeffs.getCoefficient(name);
}

std::shared_ptr<mfem::Coefficient>
CoefficientManager::getScalarCoefficientPtr(const std::string name)
{
  return this->_scalar_coeffs.getCoefficientPtr(name);
}

std::shared_ptr<mfem::VectorCoefficient>
CoefficientManager::getVectorCoefficientPtr(const std::string name)
{
  return this->_vector_coeffs.getCoefficientPtr(name);
}

std::shared_ptr<mfem::MatrixCoefficient>
CoefficientManager::getMatrixCoefficientPtr(const std::string name)
{
  return this->_matrix_coeffs.getCoefficientPtr(name);
}

bool
CoefficientManager::scalarPropertyIsDefined(const std::string & name,
                                            const std::string & block) const
{
  return this->_scalar_coeffs.propertyDefinedOnBlock(name, block);
}

bool
CoefficientManager::vectorPropertyIsDefined(const std::string & name,
                                            const std::string & block) const
{
  return this->_vector_coeffs.propertyDefinedOnBlock(name, block);
}

bool
CoefficientManager::matrixPropertyIsDefined(const std::string & name,
                                            const std::string & block) const
{
  return this->_matrix_coeffs.propertyDefinedOnBlock(name, block);
}
}
