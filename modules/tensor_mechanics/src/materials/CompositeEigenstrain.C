/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CompositeEigenstrain.h"

template <>
InputParameters
validParams<CompositeEigenstrain>()
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
