//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadratureSampler.h"
#include "Distribution.h"

registerMooseObjectAliased("StochasticToolsApp", QuadratureSampler, "Quadrature");
registerMooseObjectReplaced("StochasticToolsApp",
                            QuadratureSampler,
                            "07/01/2020 00:00",
                            Quadrature);

InputParameters
QuadratureSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Quadrature sampler for Polynomial Chaos.");
  params.addRequiredParam<unsigned int>(
      "order", "Specify the maximum order of the polynomials in the expansion.");
  MooseEnum grid("none smolyak clenshaw-curtis", "none");
  params.addParam<MooseEnum>(
      "sparse_grid", grid, "Type of sparse grid to use, if none, full tensor product is used.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix and their type defines the quadrature.");
  return params;
}

QuadratureSampler::QuadratureSampler(const InputParameters & parameters) : Sampler(parameters)
{
  // For each distribution, get the 1-D quadrature
  std::vector<std::unique_ptr<const PolynomialQuadrature::Polynomial>> poly_1d;
  for (auto dname : getParam<std::vector<DistributionName>>("distributions"))
    poly_1d.push_back(PolynomialQuadrature::makePolynomial(&getDistributionByName(dname)));

  // Here, we take the 1-D quadratures and perform a tensor product for multi-D integration
  switch (getParam<MooseEnum>("sparse_grid"))
  {
    case 0:
    {
      _grid = std::make_unique<const PolynomialQuadrature::TensorGrid>(
          getParam<unsigned int>("order") + 1, poly_1d);
      break;
    }
    case 1:
    {
      _grid = std::make_unique<const PolynomialQuadrature::SmolyakGrid>(
          getParam<unsigned int>("order"), poly_1d);
      break;
    }
    case 2:
    {
      _grid = std::make_unique<const PolynomialQuadrature::ClenshawCurtisGrid>(
          getParam<unsigned int>("order"), poly_1d);
      break;
    }
      paramError("sparse_grid", "Unknown or unimplemented sparse grid type.");
  }

  setNumberOfRows(_grid->nPoints());
  setNumberOfCols(_grid->nDim());
}

Real
QuadratureSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return _grid->quadraturePoint(row_index, col_index);
}

Real
QuadratureSampler::getQuadratureWeight(dof_id_type row_index) const
{
  return _grid->quadratureWeight(row_index);
}
