#ifdef MFEM_ENABLED

#pragma once
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/pfem_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "MFEMContainers.h"
#include "MFEMKernel.h"

namespace Moose::MFEM
{
/*
Class to store weak form components (bilinear and linear forms, and optionally
mixed and nonlinear forms) and build methods
*/
class ComplexEquationSystem : public Moose::MFEM::EquationSystem
{

public:

  ComplexEquationSystem() = default;
  ~ComplexEquationSystem() = default;

  // Complex Linear and Bilinear Forms
  Moose::MFEM::NamedFieldsMap<mfem::ParSesquilinearForm> _cslfs;
  Moose::MFEM::NamedFieldsMap<mfem::ParComplexLinearForm> _clfs;


};

}

#endif
