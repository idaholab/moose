//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralVectorPostprocessor.h"
#include "SamplerBase.h"
#include "BlockRestrictable.h"
#include "Assembly.h"
#include "MooseMesh.h"
#include "SwapBackSentinel.h"
#include "FEProblem.h"
#include "libmesh/quadrature.h"

class MooseMesh;

/**
 * Samples material properties at all quadrature points in mesh block(s)
 */
template <bool is_ad>
class Sampler1DRealTempl : public GeneralVectorPostprocessor,
                           public SamplerBase,
                           public BlockRestrictable
{
public:
  /**
   * Class constructor
   * Sets up variables for output based on the properties to be output
   * @param parameters The input parameters
   */
  Sampler1DRealTempl(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;

protected:
  /// The material properties to be output
  std::vector<const GenericMaterialProperty<Real, is_ad> *> _material_properties;

  /// The mesh
  MooseMesh & _mesh;

  /// The quadrature rule
  const QBase * const & _qrule;

  /// The quadrature points
  const MooseArray<Point> & _q_point;

public:
  static InputParameters validParams();
};

typedef Sampler1DRealTempl<false> Sampler1DReal;
typedef Sampler1DRealTempl<true> ADSampler1DReal;
