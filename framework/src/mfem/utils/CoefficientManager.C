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
CoefficientManager::declareScalar(const std::string & name, const std::string & existing_coef)
{
  this->declareScalar(name, this->_scalar_coeffs.getCoefficientPtr(existing_coef));
}

void
CoefficientManager::declareScalarProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          std::shared_ptr<mfem::Coefficient> coef)
{
  this->_scalar_coeffs.addPiecewiseBlocks(name, coef, blocks);
}

void
CoefficientManager::declareScalarProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          const std::string & existing_coef)
{
  std::shared_ptr<mfem::Coefficient> coef = this->_scalar_coeffs.getCoefficientPtr(existing_coef);
  if (std::dynamic_pointer_cast<mfem::PWCoefficient>(coef))
  {
    throw MooseException(
        "Properties must not be defined out of other properties or piecewise coefficients.");
  }
  this->declareScalarProperty(name, blocks, coef);
}

void
CoefficientManager::declareVector(const std::string & name,
                                  std::shared_ptr<mfem::VectorCoefficient> coef)
{
  this->_vector_coeffs.addCoefficient(name, coef);
}

void
CoefficientManager::declareVector(const std::string & name, const std::string & existing_coef)
{
  this->declareVector(name, this->_vector_coeffs.getCoefficientPtr(existing_coef));
}

void
CoefficientManager::declareVectorProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          std::shared_ptr<mfem::VectorCoefficient> coef)
{
  this->_vector_coeffs.addPiecewiseBlocks(name, coef, blocks);
}

void
CoefficientManager::declareVectorProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          const std::string & existing_coef)
{
  std::shared_ptr<mfem::VectorCoefficient> coef =
      this->_vector_coeffs.getCoefficientPtr(existing_coef);
  if (std::dynamic_pointer_cast<mfem::PWVectorCoefficient>(coef))
  {
    throw MooseException(
        "Properties must not be defined out of other properties or piecewise coefficients.");
  }
  this->declareVectorProperty(name, blocks, coef);
}

void
CoefficientManager::declareMatrix(const std::string & name,
                                  std::shared_ptr<mfem::MatrixCoefficient> coef)
{
  this->_matrix_coeffs.addCoefficient(name, coef);
}

void
CoefficientManager::declareMatrix(const std::string & name, const std::string & existing_coef)
{
  this->declareMatrix(name, this->_matrix_coeffs.getCoefficientPtr(existing_coef));
}

void
CoefficientManager::declareMatrixProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          std::shared_ptr<mfem::MatrixCoefficient> coef)
{
  this->_matrix_coeffs.addPiecewiseBlocks(name, coef, blocks);
}

void
CoefficientManager::declareMatrixProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          const std::string & existing_coef)
{
  std::shared_ptr<mfem::MatrixCoefficient> coef =
      this->_matrix_coeffs.getCoefficientPtr(existing_coef);
  if (std::dynamic_pointer_cast<mfem::PWMatrixCoefficient>(coef))
  {
    throw MooseException(
        "Properties must not be defined out of other properties or piecewise coefficients.");
  }
  this->declareMatrixProperty(name, blocks, coef);
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
