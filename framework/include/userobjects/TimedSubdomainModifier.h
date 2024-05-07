//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementSubdomainModifier.h"
#include "TimedElementSubdomainModifier.h"
#include "DelimitedFileReader.h"

/**
 * Modifies elements from entire subdomains based on user input or file input
 */
class TimedSubdomainModifier : public TimedElementSubdomainModifier
{
public:
  static InputParameters validParams();

  TimedSubdomainModifier(const InputParameters & parameters);

  virtual void initialize() override;

protected:
  /**
   * Requests a vector of all points in time from the inheriting class
   * (these do not have to be sorted).
   * @returns Unsorted vector of points in time.
   */
  virtual std::vector<Real> onGetTimes() override;

  /**
   * Determines the new subdomain-id for the current element.
   * @returns New subdomain id to move the element to.
   */
  virtual SubdomainID onComputeSubdomainID(Real t_from_exclusive, Real t_to_inclusive) override;

private:
  void buildFromParameters();
  void buildFromFile();

  /// Times to change the subdomains on. If the time steps do not align with the times,
  /// the subdomain changes will happen at the end of the time step
  std::vector<Real> _times;

  /// Source subdomains to change from
  std::vector<SubdomainID> _blocks_from;
  /// Target subdomains to change the source subdomains to
  std::vector<SubdomainID> _blocks_to;
};
