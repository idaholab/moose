/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CompositeEigenstrain.h"

template<>
InputParameters validParams<CompositeEigenstrain>()
{
  InputParameters params = CompositeTensorBase<RankTwoTensor, ComputeStressFreeStrainBase>::validParams();
  params.addClassDescription("Assemble an Eigenstrain tensor from multiple tensor contributions weighted by material properties");
  return params;
}

CompositeEigenstrain::CompositeEigenstrain(const InputParameters & parameters) :
    CompositeTensorBase<RankTwoTensor, ComputeStressFreeStrainBase>(parameters)
{
  initializeDerivativeProperties(_base_name + "elastic_strain");
}

void
CompositeEigenstrain::computeQpStressFreeStrain()
{
  // Define Eigenstrain (and fill in the derivatives of elastic strain with a prefactor of -1)
  computeQpTensorProperties(_stress_free_strain, -1.0);
}
