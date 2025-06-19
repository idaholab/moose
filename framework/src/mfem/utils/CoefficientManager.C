//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MFEM_ENABLED

#include "MooseStringUtils.h"
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
CoefficientManager::declareScalar(const std::string & name, const std::string & existing_or_literal)
{
  return this->declareScalar(name, this->getScalarCoefficientPtr(existing_or_literal));
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
                                          const std::string & existing_or_literal)
{
  std::shared_ptr<mfem::Coefficient> coef = this->getScalarCoefficientPtr(existing_or_literal);
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
CoefficientManager::declareVector(const std::string & name, const std::string & existing_or_literal)
{
  return this->declareVector(name, this->getVectorCoefficientPtr(existing_or_literal));
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
                                          const std::string & existing_or_literal)
{
  std::shared_ptr<mfem::VectorCoefficient> coef =
      this->getVectorCoefficientPtr(existing_or_literal);
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
  return this->declareMatrix(name, this->getMatrixCoefficientPtr(existing_coef));
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
  std::shared_ptr<mfem::MatrixCoefficient> coef = this->getMatrixCoefficientPtr(existing_coef);
  if (std::dynamic_pointer_cast<mfem::PWMatrixCoefficient>(coef))
    mooseError("Properties must not be defined out of other properties or piecewise coefficients.");
  return this->declareMatrixProperty(name, blocks, coef);
}

std::shared_ptr<mfem::Coefficient>
CoefficientManager::getScalarCoefficientPtr(const std::string & name)
{
  if (this->_scalar_coeffs.hasCoefficient(name))
    return this->_scalar_coeffs.getCoefficientPtr(name);
  // If name not present, check if it can be parsed cleanly into a real number
  std::istringstream ss(MooseUtils::trim(name));
  mfem::real_t real_value;
  if (ss >> real_value && ss.eof())
  {
    this->declareScalar<mfem::ConstantCoefficient>(name, real_value);
    return this->_scalar_coeffs.getCoefficientPtr(name);
  }
  mooseError("Scalar coefficient with name '" + name + "' has not been declared.");
}

std::shared_ptr<mfem::VectorCoefficient>
CoefficientManager::getVectorCoefficientPtr(const std::string & name)
{
  if (this->_vector_coeffs.hasCoefficient(name))
    return this->_vector_coeffs.getCoefficientPtr(name);
  // If name not present, check if it can be parsed cleanly into a vector of real numbers
  std::vector<mfem::real_t> vec_values;
  if (MooseUtils::tokenizeAndConvert(name, vec_values) && vec_values.size() > 0)
  {
    this->declareVector<mfem::VectorConstantCoefficient>(
        name, mfem::Vector(vec_values.data(), vec_values.size()));
    return this->_vector_coeffs.getCoefficientPtr(name);
  }
  mooseError("Vector oefficient with name '" + name + "' has not been declared.");
}

std::shared_ptr<mfem::MatrixCoefficient>
CoefficientManager::getMatrixCoefficientPtr(const std::string & name)
{
  return this->_matrix_coeffs.getCoefficientPtr(name);
  // TODO: Work out how to parse literal matrices from input.
}

mfem::Coefficient &
CoefficientManager::getScalarCoefficient(const std::string & name)
{
  return *this->getScalarCoefficientPtr(name);
}

mfem::VectorCoefficient &
CoefficientManager::getVectorCoefficient(const std::string & name)
{
  return *this->getVectorCoefficientPtr(name);
}

mfem::MatrixCoefficient &
CoefficientManager::getMatrixCoefficient(const std::string & name)
{
  return *this->getMatrixCoefficientPtr(name);
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
