//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideVectorPostprocessor.h"
#include "SamplerBase.h"
#include "NonADFunctorInterface.h"

class InterfaceNusseltSampler : public SideVectorPostprocessor,
	                        protected SamplerBase,
			       	public NonADFunctorInterface
{
public:
  static InputParameters validParams();

  InterfaceNusseltSampler(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

  // Let the SamplerBase version of threadJoin() take part in the
  // overload resolution process, otherwise we get warnings about
  // overloaded virtual functions and "hiding" in debug mode.
  using SamplerBase::threadJoin;

  virtual void threadJoin(const UserObject & y) override;

protected:
  /// So we don't have to create and destroy this vector over and over again
  std::vector<Real> _values;

private:
  /// The fluid temperature used for the Nusselt number calculation
  const Moose::Functor<Real> * _fluid_temp;

  /// Characteristic length
  const Moose::Functor<Real> &_char_length;

  /// bulk temperature value
  const Moose::Functor<Real> &_bulk_temp;
};
