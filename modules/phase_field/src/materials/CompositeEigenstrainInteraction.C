//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CompositeEigenstrainInteraction.h"

registerMooseObject("TensorMechanicsApp", CompositeEigenstrainInteraction);

InputParameters
CompositeEigenstrainInteraction::validParams()
{
  InputParameters params =
      CompositeTensorBase<RankTwoTensor, ComputeEigenstrainInteractionBase>::validParams();
  params.addClassDescription("Assemble an Eigenstrain Interaction tensor from multiple tensor contributions "
                             "weighted by material properties");
  return params;
}

CompositeEigenstrainInteraction::CompositeEigenstrainInteraction(const InputParameters & parameters)
  : CompositeTensorBase<RankTwoTensor, ComputeEigenstrainInteractionBase>(parameters)
{
  initializeDerivativeProperties(_base_name + "elastic_strain_int");
}

void
CompositeEigenstrainInteraction::computeQpEigenstrainInteraction()
{
  // Define Eigenstrain Interaction (and fill in the derivatives of elastic strain with a prefactor of -1)
  computeQpTensorProperties(_eigenstrainInteraction, -1.0);
}
