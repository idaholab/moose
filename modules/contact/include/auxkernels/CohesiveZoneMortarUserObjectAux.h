//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include "MooseEnum.h"

#include <functional>
#include <map>
#include <tuple>
#include <string>

class UserObject;
class BilinearMixedModeCohesiveZoneModel;

/**
 * Auxiliary kernel to output mortar penalty contact quantities of interest
 */
class CohesiveZoneMortarUserObjectAux : public AuxKernel
{
public:
  static InputParameters validParams();

  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the
   * constructor.
   */
  CohesiveZoneMortarUserObjectAux(const InputParameters & parameters);

protected:
  /// What type of cohesive zone quantity we are querying
  enum class CohesiveQuantityEnum
  {
    MODE_MIXITY_RATIO,
    COHESIVE_DAMAGE
  };

  virtual Real computeValue() override;

  /// What penalty mortar contact quantity we'd like to output
  const CohesiveQuantityEnum _cohesive_zone_quantity;

  /// The user object inputted by the user to obtain the cohesive zone quantities
  const UserObject & _user_object;

  ///@{ Cast pointers to specific UOs
  const BilinearMixedModeCohesiveZoneModel * _cohesive_zone_uo;
  ///@}

  /// Definition of the output quantities and
  std::map<CohesiveQuantityEnum, std::tuple<std::string, const void *, std::function<Real(void)>>>
      _outputs;

  /// if true computeValue only performs error checking (used in constructor)
  bool _check_only;

  /// Available cohesive zone model quantities
  static const MooseEnum _cohesive_zone_quantities;
};
