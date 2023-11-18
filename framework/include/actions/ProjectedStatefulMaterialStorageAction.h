//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "libmesh/fe_type.h"

/**
 * Set up AuxKernels and AuxVariables for projected material property storage (PoMPS).
 */
class ProjectedStatefulMaterialStorageAction : public Action
{
public:
  static InputParameters validParams();

  ProjectedStatefulMaterialStorageAction(const InputParameters & parameters);

  virtual void act() override;

  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

  /// List of supported types
  typedef Moose::TypeList<Real, RealVectorValue, RankTwoTensor, RankFourTensor> SupportedTypes;

protected:
  /**
   * Perform setup for a single scalar component of the material property prop_name, and gather the
   * names of teh AuxVariables used to represent each component in `vars`.
   */
  void processComponent(const std::string & prop_name,
                        std::vector<unsigned int> idx,
                        std::vector<VariableName> & vars,
                        bool is_ad);

  /**
   * Add the material object to obtain the interpolated old state (for use with
   * InterpolatedStatefulMaterialPropertyInterface)
   */
  void addMaterial(const std::string & prop_type,
                   const std::string & prop_name,
                   std::vector<VariableName> & vars);

  enum class PropertyType
  {
    REAL,
    REALVECTORVALUE,
    RANKTWOTENSOR,
    RANKFOURTENSOR
  };
  typedef std::pair<PropertyType, bool> PropertyInfo;

  /**
   * Return the property type and whether to use AD or not. If no property with a supported type is
   * found, throw an error.
   */
  PropertyInfo checkProperty(const std::string & prop_name);

  const std::vector<MaterialPropertyName> & _prop_names;

  const MooseEnum _order;
  FEType _fe_type;
  const std::string _var_type;
  const std::string _pomps_prefix;
};
