#pragma once

#include "GeneralUserObject.h"
#include "boundary_conditions.h"
#include "gridfunctions.h"
#include "coefficients.h"
#include "Function.h"

class MFEMBoundaryCondition : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMBoundaryCondition(const InputParameters & parameters);
  ~MFEMBoundaryCondition() override {}

  inline virtual std::shared_ptr<platypus::BoundaryCondition> getBC() const
  {
    return _boundary_condition;
  }

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

protected:
  std::vector<BoundaryName> _boundary_names;
  mfem::Array<int> bdr_attr;

  std::shared_ptr<platypus::BoundaryCondition> _boundary_condition{nullptr};
};
