#include "PropertyManager.h"

namespace platypus
{
;

template <class T, class Tpw>
inline void
declareCoefficient(PropertyMap<T, Tpw> & map,
                   const std::string & name,
                   std::shared_ptr<T> coef,
                   const std::vector<std::string> & blocks)
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
PropertyManager::declareScalar(const std::string & name,
                               mfem::real_t value,
                               const std::vector<std::string> & blocks)
{
  this->declareScalar(name, this->_scalar_manager.make<mfem::ConstantCoefficient>(value), blocks);
}

void
PropertyManager::declareScalar(const std::string & name,
                               std::function<mfem::real_t(const mfem::Vector &)> func,
                               const std::vector<std::string> & blocks)
{
  this->declareScalar(name, _scalar_manager.make<mfem::FunctionCoefficient>(func), blocks);
}

void
PropertyManager::declareScalar(
    const std::string & name,
    std::function<mfem::real_t(const mfem::Vector &, mfem::real_t t)> func,
    const std::vector<std::string> & blocks)
{
  this->declareScalar(name, this->_scalar_manager.make<mfem::FunctionCoefficient>(func), blocks);
}

void
PropertyManager::declareScalar(const std::string & name,
                               std::shared_ptr<mfem::Coefficient> coef,
                               const std::vector<std::string> & blocks)
{
  declareCoefficient(this->_scalar_coeffs, name, coef, blocks);
}

void
PropertyManager::declareVector(const std::string & name,
                               const mfem::Vector & value,
                               const std::vector<std::string> & blocks)
{
  this->declareVector(
      name, this->_vector_manager.make<mfem::VectorConstantCoefficient>(value), blocks);
}

void
PropertyManager::declareVector(const std::string & name,
                               int dim,
                               std::function<void(const mfem::Vector &, mfem::Vector &)> func,
                               const std::vector<std::string> & blocks)
{
  this->declareVector(
      name, this->_vector_manager.make<mfem::VectorFunctionCoefficient>(dim, func), blocks);
}

void
PropertyManager::declareVector(
    const std::string & name,
    int dim,
    std::function<void(const mfem::Vector &, mfem::real_t t, mfem::Vector &)> func,
    const std::vector<std::string> & blocks)
{
  this->declareVector(
      name, this->_vector_manager.make<mfem::VectorFunctionCoefficient>(dim, func), blocks);
}

void
PropertyManager::declareVector(const std::string & name,
                               std::shared_ptr<mfem::VectorCoefficient> coef,
                               const std::vector<std::string> & blocks)
{
  declareCoefficient(this->_vector_coeffs, name, coef, blocks);
}

void
PropertyManager::declareMatrix(const std::string & name,
                               const mfem::DenseMatrix & value,
                               const std::vector<std::string> & blocks)
{
  this->declareMatrix(
      name, this->_matrix_manager.make<mfem::MatrixConstantCoefficient>(value), blocks);
}

void
PropertyManager::declareMatrix(const std::string & name,
                               int dim,
                               std::function<void(const mfem::Vector &, mfem::DenseMatrix &)> func,
                               const std::vector<std::string> & blocks)
{
  this->declareMatrix(
      name, this->_matrix_manager.make<mfem::MatrixFunctionCoefficient>(dim, func), blocks);
}

void
PropertyManager::declareMatrix(
    const std::string & name,
    int dim,
    std::function<void(const mfem::Vector &, mfem::real_t, mfem::DenseMatrix &)> func,
    const std::vector<std::string> & blocks)
{
  this->declareMatrix(
      name, this->_matrix_manager.make<mfem::MatrixFunctionCoefficient>(dim, func), blocks);
}

void
PropertyManager::declareMatrix(const std::string & name,
                               std::shared_ptr<mfem::MatrixCoefficient> coef,
                               const std::vector<std::string> & blocks)
{
  declareCoefficient(this->_matrix_coeffs, name, coef, blocks);
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
