//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "LibtorchArtificialNeuralNetTrainer.h"

namespace Moose
{
// Explicitly instantiate for Random and Sequential samplers
template class LibtorchArtificialNeuralNetTrainer<torch::data::samplers::DistributedRandomSampler>;

template class LibtorchArtificialNeuralNetTrainer<
    torch::data::samplers::DistributedSequentialSampler>;
}

#endif
