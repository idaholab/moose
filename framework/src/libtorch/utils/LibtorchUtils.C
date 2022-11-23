//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED
#include "LibtorchUtils.h"

namespace LibtorchUtils
{

template void
vectorToTensor<Real>(std::vector<Real> & vector, torch::Tensor & tensor, const bool detach);

template void tensorToVector<Real>(torch::Tensor & tensor, std::vector<Real> & vector);

} // LibtorchUtils namespace
#endif
