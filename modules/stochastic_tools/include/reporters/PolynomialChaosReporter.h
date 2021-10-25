//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "SurrogateModelInterface.h"

#include "PolynomialChaos.h"
#include "nlohmann/json.h"

class PolynomialChaosReporter : public GeneralReporter, public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  PolynomialChaosReporter(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

private:
  /// Helper function for computing local sensitivity from a polynomial chaos model
  static std::vector<Real> computeLocalSensitivity(const PolynomialChaos & pc,
                                                   const std::vector<Real> & data);

  /// Points for local sensitivity calculation
  const std::vector<std::vector<Real>> & _loc_point;
  /// Samplers for local sensitivity calculation
  std::vector<Sampler *> _loc_sampler;

  /// Polynomial chaos models
  std::vector<const PolynomialChaos *> _pc;

  /// Local sensitivity from specified points
  std::vector<std::vector<std::vector<Real>> *> _loc_point_sense;
  /// Local sensitivity from sampled points
  std::vector<std::vector<std::vector<Real>> *> _loc_sampler_sense;
};

void to_json(nlohmann::json & json, const PolynomialChaos * const & pc);

/**
 * PCStatisticsContext is almost identical to ReporterStatisticsContext with
 * InType == Outype. Unfortunately, we cannot derive from ReporterStatisticsContext
 * since that class relies on the construction of a Calculator object, something
 * that is unnecessary for calculating statistics with polynomial chaos.
 */
template <typename OutType>
class PCStatisticsContext : public ReporterGeneralContext<std::pair<OutType, std::vector<OutType>>>
{
public:
  PCStatisticsContext(const libMesh::ParallelObject & other,
                      const MooseObject & producer,
                      ReporterState<std::pair<OutType, std::vector<OutType>>> & state,
                      const PolynomialChaos & pc,
                      const MooseEnumItem & stat);

  virtual void finalize() override;
  virtual void storeInfo(nlohmann::json & json) const override;

private:
  /// Polynomial chaos surrogate object
  const PolynomialChaos & _pc;
  /// The stat to compute
  const MooseEnumItem _stat;
};

/**
 * PCSobolContext is almost identical to SobolReporterContext with
 * InType == Outype. Unfortunately, we cannot derive from SobolReporterContext
 * since that class relies on the construction of a Calculator object, something
 * that is unnecessary for calculating statistics with polynomial chaos.
 */
template <typename OutType>
class PCSobolContext : public ReporterGeneralContext<
                           std::pair<std::vector<OutType>, std::vector<std::vector<OutType>>>>
{
public:
  PCSobolContext(
      const libMesh::ParallelObject & other,
      const MooseObject & producer,
      ReporterState<std::pair<std::vector<OutType>, std::vector<std::vector<OutType>>>> & state,
      const PolynomialChaos & pc);

  virtual void finalize() override;
  virtual std::string type() const override
  {
    return "SobolIndices<" + MooseUtils::prettyCppType<OutType>() + ">";
  }

protected:
  virtual void store(nlohmann::json & json) const override;

private:
  /// Polynomial chaos surrogate object
  const PolynomialChaos & _pc;
};
