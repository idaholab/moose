#include "MFEMAuxSolver.h"

registerMooseObject("PlatypusApp", MFEMAuxSolver);

InputParameters
MFEMAuxSolver::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  // NB: register as base. MFEM auxkernels should be derived from this base class. This allows the
  // same AuxKernel block to be used to setup both MFEM auxsolvers and MOOSE auxkernels.
  params.registerBase("MFEMAuxKernel");

  return params;
}

MFEMAuxSolver::MFEMAuxSolver(const InputParameters & parameters) : GeneralUserObject(parameters) {}

MFEMAuxSolver::~MFEMAuxSolver() {}
