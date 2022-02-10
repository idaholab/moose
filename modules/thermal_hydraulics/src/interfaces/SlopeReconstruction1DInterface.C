//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SlopeReconstruction1DInterface.h"

template <>
typename SlopeReconstruction1DInterface<true>::ESlopeReconstructionType
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<SlopeReconstruction1DInterface<true>::ESlopeReconstructionType>(
      s, SlopeReconstruction1DInterface<true>::_slope_reconstruction_type_to_enum);
}

template <>
typename SlopeReconstruction1DInterface<false>::ESlopeReconstructionType
THM::stringToEnum(const std::string & s)
{
  return stringToEnum<SlopeReconstruction1DInterface<false>::ESlopeReconstructionType>(
      s, SlopeReconstruction1DInterface<false>::_slope_reconstruction_type_to_enum);
}
