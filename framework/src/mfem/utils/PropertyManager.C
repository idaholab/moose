#include "PropertyManager.h"

namespace platypus
{
;

template <class T, class Tpw>
void
declareCoefficient(PropertyMap<T, Tpw> & map,
                   const std::string & name,
                   std::unique_ptr<T> && coef,
                   const std::vector<std::string> & blocks)
{
  if (blocks.empty())
  {
    map.addProperty(name, std::move(coef));
  }
  else
  {
    map.addPiecewiseBlocks(name, std::shared_ptr<T>(std::move(coef)), blocks);
  }
}

void
PropertyManager::declareScalar(const std::string & name,
                               mfem::real_t value,
                               const std::vector<std::string> & blocks)
{
  this->declareScalar(name, std::make_unique<mfem::ConstantCoefficient>(value), blocks);
}

void
PropertyManager::declareScalar(const std::string & name,
                               std::function<mfem::real_t(const mfem::Vector &)> func,
                               const std::vector<std::string> & blocks)
{
  this->declareScalar(name, std::make_unique<mfem::FunctionCoefficient>(func), blocks);
}

void
PropertyManager::declareScalar(
    const std::string & name,
    std::function<mfem::real_t(const mfem::Vector &, mfem::real_t t)> func,
    const std::vector<std::string> & blocks)
{
  this->declareScalar(name, std::make_unique<mfem::FunctionCoefficient>(func), blocks);
}

void
PropertyManager::declareScalar(const std::string & name,
                               std::unique_ptr<mfem::Coefficient> && coef,
                               const std::vector<std::string> & blocks)
{
  declareCoefficient(this->_scalar_coeffs, name, std::move(coef), blocks);
}

void
PropertyManager::declareVector(const std::string & name,
                               const mfem::Vector & value,
                               const std::vector<std::string> & blocks)
{
  this->declareVector(name, std::make_unique<mfem::VectorConstantCoefficient>(value), blocks);
}

void
PropertyManager::declareVector(const std::string & name,
                               int dim,
                               std::function<void(const mfem::Vector &, mfem::Vector &)> func,
                               const std::vector<std::string> & blocks)
{
  this->declareVector(name, std::make_unique<mfem::VectorFunctionCoefficient>(dim, func), blocks);
}

void
PropertyManager::declareVector(
    const std::string & name,
    int dim,
    std::function<void(const mfem::Vector &, mfem::real_t t, mfem::Vector &)> func,
    const std::vector<std::string> & blocks)
{
  this->declareVector(name, std::make_unique<mfem::VectorFunctionCoefficient>(dim, func), blocks);
}

void
PropertyManager::declareVector(const std::string & name,
                               std::unique_ptr<mfem::VectorCoefficient> && coef,
                               const std::vector<std::string> & blocks)
{
  declareCoefficient(this->_vector_coeffs, name, std::move(coef), blocks);
}

void
PropertyManager::declareMatrix(const std::string & name,
                               const mfem::DenseMatrix & value,
                               const std::vector<std::string> & blocks)
{
  this->declareMatrix(name, std::make_unique<mfem::MatrixConstantCoefficient>(value), blocks);
}

void
PropertyManager::declareMatrix(const std::string & name,
                               int dim,
                               std::function<void(const mfem::Vector &, mfem::DenseMatrix &)> func,
                               const std::vector<std::string> & blocks)
{
  this->declareMatrix(name, std::make_unique<mfem::MatrixFunctionCoefficient>(dim, func), blocks);
}

void
PropertyManager::declareMatrix(
    const std::string & name,
    int dim,
    std::function<void(const mfem::Vector &, mfem::real_t, mfem::DenseMatrix &)> func,
    const std::vector<std::string> & blocks)
{
  this->declareMatrix(name, std::make_unique<mfem::MatrixFunctionCoefficient>(dim, func), blocks);
}

void
PropertyManager::declareMatrix(const std::string & name,
                               std::unique_ptr<mfem::MatrixCoefficient> && coef,
                               const std::vector<std::string> & blocks)
{
  declareCoefficient(this->_matrix_coeffs, name, std::move(coef), blocks);
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
