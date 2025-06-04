#ifdef MFEM_ENABLED

#include "MFEMProblemData.h"

void
MFEMProblemData::updateFESpaces()
{
  for (const auto & fe_space_pair : fespaces)
  {
    fe_space_pair.second->Update();
  }
  for (const auto & gridfunction_pair : gridfunctions)
  {
    gridfunction_pair.second->Update();
  }
  eqn_system->UpdateEquationSystem();
}

#endif
