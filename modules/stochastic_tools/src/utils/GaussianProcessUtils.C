//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GaussianProcessUtils.h"

namespace StochasticTools
{

void
GaussianProcessUtils::mapToPetscVec(
    const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
        tuning_data,
    const std::unordered_map<std::string, Real> & scalar_map,
    const std::unordered_map<std::string, std::vector<Real>> & vector_map,
    libMesh::PetscVector<Number> & petsc_vec)
{
  for (auto iter = tuning_data.begin(); iter != tuning_data.end(); ++iter)
  {
    std::string param_name = iter->first;
    const auto scalar_it = scalar_map.find(param_name);
    if (scalar_it != scalar_map.end())
      petsc_vec.set(std::get<0>(iter->second), scalar_it->second);
    else
    {
      const auto vector_it = vector_map.find(param_name);
      if (vector_it != vector_map.end())
        for (unsigned int ii = 0; ii < std::get<1>(iter->second); ++ii)
          petsc_vec.set(std::get<0>(iter->second) + ii, (vector_it->second)[ii]);
    }
  }
}

void
GaussianProcessUtils::petscVecToMap(
    const std::unordered_map<std::string, std::tuple<unsigned int, unsigned int, Real, Real>> &
        tuning_data,
    std::unordered_map<std::string, Real> & scalar_map,
    std::unordered_map<std::string, std::vector<Real>> & vector_map,
    const libMesh::PetscVector<Number> & petsc_vec)
{
  for (auto iter = tuning_data.begin(); iter != tuning_data.end(); ++iter)
  {
    std::string param_name = iter->first;
    if (scalar_map.find(param_name) != scalar_map.end())
      scalar_map[param_name] = petsc_vec(std::get<0>(iter->second));
    else if (vector_map.find(param_name) != vector_map.end())
      for (unsigned int ii = 0; ii < std::get<1>(iter->second); ++ii)
        vector_map[param_name][ii] = petsc_vec(std::get<0>(iter->second) + ii);
  }
}

void
GaussianProcessUtils::setupStoredMatrices(const RealEigenMatrix & input)
{
  _K_cho_decomp = _K.llt();
  _K_results_solve = _K_cho_decomp.solve(input);
}

} // StochasticTools namespace

template <>
void
dataStore(std::ostream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context)
{
  // Store the L matrix as opposed to the full matrix to avoid compounding
  // roundoff error and decomposition error
  RealEigenMatrix L(decomp.matrixL());
  dataStore(stream, L, context);
}

template <>
void
dataLoad(std::istream & stream, Eigen::LLT<RealEigenMatrix> & decomp, void * context)
{
  RealEigenMatrix L;
  dataLoad(stream, L, context);
  decomp.compute(L * L.transpose());
}

template <>
void
dataStore(std::ostream & stream, StochasticTools::GaussianProcessUtils & gp_utils, void * context)
{
  dataStore(stream, gp_utils.K(), context);
  dataStore(stream, gp_utils.KResultsSolve(), context);
  dataStore(stream, gp_utils.KCholeskyDecomp(), context);
  dataStore(stream, gp_utils.paramStandardizer(), context);
  dataStore(stream, gp_utils.dataStandardizer(), context);
}

template <>
void
dataLoad(std::istream & stream, StochasticTools::GaussianProcessUtils & gp_utils, void * context)
{
  dataLoad(stream, gp_utils.K(), context);
  dataLoad(stream, gp_utils.KResultsSolve(), context);
  dataLoad(stream, gp_utils.KCholeskyDecomp(), context);
  dataLoad(stream, gp_utils.paramStandardizer(), context);
  dataLoad(stream, gp_utils.dataStandardizer(), context);
}
