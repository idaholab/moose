//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "GeneralPostprocessor.h"
#include "FaceCenteredMapFunctor.h"

#include <unordered_map>

class SharpInterfacePressureWritebackReconstructionError : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  SharpInterfacePressureWritebackReconstructionError(const InputParameters & parameters);

  void initialize() override;
  void execute() override {}
  void finalize() override;
  PostprocessorValue getValue() const override;

protected:
  const RealVectorValue _constant_vector;
  PostprocessorValue _reconstruction_error;
  FaceCenteredMapFunctor<RealVectorValue, std::unordered_map<dof_id_type, RealVectorValue>>
      _face_values;
};
