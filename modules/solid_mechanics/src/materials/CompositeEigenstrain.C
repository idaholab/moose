//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CompositeEigenstrain.h"

registerMooseObject("TensorMechanicsApp", CompositeEigenstrain);

InputParameters
CompositeEigenstrain::validParams()
{
  InputParameters params =
      CompositeTensorBase<RankTwoTensor, ComputeEigenstrainBase>::validParams();
  params.addClassDescription("Assemble an Eigenstrain tensor from multiple tensor contributions "
                             "weighted by material properties");
  return params;
}

CompositeEigenstrain::CompositeEigenstrain(const InputParameters & parameters)
  : CompositeTensorBase<RankTwoTensor, ComputeEigenstrainBase>(parameters)
{
  initializeDerivativeProperties(_base_name + "elastic_strain");
}

void
CompositeEigenstrain::computeQpEigenstrain()
{
  // Define Eigenstrain (and fill in the derivatives of elastic strain with a prefactor of -1)
  computeQpTensorProperties(_eigenstrain, -1.0);
}
