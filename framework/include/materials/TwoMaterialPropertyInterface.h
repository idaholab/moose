/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef TWOMATERIALPROPERTYINTERFACE_H
#define TWOMATERIALPROPERTYINTERFACE_H

#include "MaterialPropertyInterface.h"

// Forward Declarations
class MaterialData;
class TwoMaterialPropertyInterface;

template <>
InputParameters validParams<TwoMaterialPropertyInterface>();

class TwoMaterialPropertyInterface : public MaterialPropertyInterface
{
public:
  ///@{
  /**
   * Constructor.
   *
   * @param parameters The objects input parameters
   * @param block_ids A reference to the block ids (optional)
   *
   * This class has two constructors:
   *   (1) not restricted to boundaries or blocks
   *   (2) restricted to only blocks
   */
  TwoMaterialPropertyInterface(const MooseObject * moose_object);
  TwoMaterialPropertyInterface(const MooseObject * moose_object,
                               const std::set<SubdomainID> & block_ids);
  ///@}

  /**
   * Retrieve the property named "name"
   */
  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialProperty(const std::string & name);

  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyOld(const std::string & name);

  template <typename T>
  const MaterialProperty<T> & getNeighborMaterialPropertyOlder(const std::string & name);

protected:
  std::shared_ptr<MaterialData> _neighbor_material_data;
};

template <typename T>
const MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialProperty(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;
  else
    return _neighbor_material_data->getProperty<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOld(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;
  else
    return _neighbor_material_data->getPropertyOld<T>(prop_name);
}

template <typename T>
const MaterialProperty<T> &
TwoMaterialPropertyInterface::getNeighborMaterialPropertyOlder(const std::string & name)
{
  // Check if the supplied parameter is a valid input parameter key
  std::string prop_name = deducePropertyName(name);

  // Check if it's just a constant
  const MaterialProperty<T> * default_property = defaultMaterialProperty<T>(prop_name);
  if (default_property)
    return *default_property;
  else
    return _neighbor_material_data->getPropertyOlder<T>(prop_name);
}

#endif // TWOMATERIALPROPERTYINTERFACE_H
