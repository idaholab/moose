#pragma once

#include "GeneralUserObject.h"
#include "boundary_conditions.hpp"
#include "gridfunctions.hpp"
#include "coefficients.hpp"
#include "Function.h"

class MFEMBoundaryCondition : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MFEMBoundaryCondition(const InputParameters & parameters);
  ~MFEMBoundaryCondition() override {}

  inline virtual std::shared_ptr<hephaestus::BoundaryCondition> getBC() const
  {
    return _boundary_condition;
  }

  virtual void execute() override {}
  virtual void initialize() override {}
  virtual void finalize() override {}

protected:
  std::vector<BoundaryName> _boundary_names;
  mfem::Array<int> bdr_attr;

  std::shared_ptr<hephaestus::BoundaryCondition> _boundary_condition{nullptr};
};
