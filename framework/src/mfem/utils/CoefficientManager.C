#ifdef MFEM_ENABLED

#include "CoefficientManager.h"
#include <algorithm>

namespace Moose::MFEM
{

mfem::Coefficient &
CoefficientManager::declareScalar(const std::string & name, std::shared_ptr<mfem::Coefficient> coef)
{
  this->_scalar_coeffs.addCoefficient(name, coef);
  return *coef;
}

mfem::Coefficient &
CoefficientManager::declareScalar(const std::string & name, const std::string & existing_coef)
{
  return this->declareScalar(name, this->_scalar_coeffs.getCoefficientPtr(existing_coef));
}

mfem::Coefficient &
CoefficientManager::declareScalarProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          std::shared_ptr<mfem::Coefficient> coef)
{
  this->_scalar_coeffs.addPiecewiseBlocks(name, coef, blocks);
  return getScalarCoefficient(name);
}

mfem::Coefficient &
CoefficientManager::declareScalarProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          const std::string & existing_coef)
{
  std::shared_ptr<mfem::Coefficient> coef = this->_scalar_coeffs.getCoefficientPtr(existing_coef);
  if (std::dynamic_pointer_cast<mfem::PWCoefficient>(coef))
    mooseError("Properties must not be defined out of other properties or piecewise coefficients.");
  return this->declareScalarProperty(name, blocks, coef);
}

mfem::VectorCoefficient &
CoefficientManager::declareVector(const std::string & name,
                                  std::shared_ptr<mfem::VectorCoefficient> coef)
{
  this->_vector_coeffs.addCoefficient(name, coef);
  return *coef;
}

mfem::VectorCoefficient &
CoefficientManager::declareVector(const std::string & name, const std::string & existing_coef)
{
  return this->declareVector(name, this->_vector_coeffs.getCoefficientPtr(existing_coef));
}

mfem::VectorCoefficient &
CoefficientManager::declareVectorProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          std::shared_ptr<mfem::VectorCoefficient> coef)
{
  this->_vector_coeffs.addPiecewiseBlocks(name, coef, blocks);
  return getVectorCoefficient(name);
}

mfem::VectorCoefficient &
CoefficientManager::declareVectorProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          const std::string & existing_coef)
{
  std::shared_ptr<mfem::VectorCoefficient> coef =
      this->_vector_coeffs.getCoefficientPtr(existing_coef);
  if (std::dynamic_pointer_cast<mfem::PWVectorCoefficient>(coef))
    mooseError("Properties must not be defined out of other properties or piecewise coefficients.");
  return this->declareVectorProperty(name, blocks, coef);
}

mfem::MatrixCoefficient &
CoefficientManager::declareMatrix(const std::string & name,
                                  std::shared_ptr<mfem::MatrixCoefficient> coef)
{
  this->_matrix_coeffs.addCoefficient(name, coef);
  return *coef;
}

mfem::MatrixCoefficient &
CoefficientManager::declareMatrix(const std::string & name, const std::string & existing_coef)
{
  return this->declareMatrix(name, this->_matrix_coeffs.getCoefficientPtr(existing_coef));
}

mfem::MatrixCoefficient &
CoefficientManager::declareMatrixProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          std::shared_ptr<mfem::MatrixCoefficient> coef)
{
  this->_matrix_coeffs.addPiecewiseBlocks(name, coef, blocks);
  return getMatrixCoefficient(name);
}

mfem::MatrixCoefficient &
CoefficientManager::declareMatrixProperty(const std::string & name,
                                          const std::vector<std::string> & blocks,
                                          const std::string & existing_coef)
{
  std::shared_ptr<mfem::MatrixCoefficient> coef =
      this->_matrix_coeffs.getCoefficientPtr(existing_coef);
  if (std::dynamic_pointer_cast<mfem::PWMatrixCoefficient>(coef))
    mooseError("Properties must not be defined out of other properties or piecewise coefficients.");
  return this->declareMatrixProperty(name, blocks, coef);
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

void
CoefficientManager::setTime(const double time)
{
  this->_scalar_coeffs.setTime(time);
  this->_vector_coeffs.setTime(time);
  this->_matrix_coeffs.setTime(time);
}
}

#endif
