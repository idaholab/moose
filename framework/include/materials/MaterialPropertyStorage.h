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

#ifndef MATERIALPROPERTYSTORAGE_H
#define MATERIALPROPERTYSTORAGE_H

#include "elem.h"
#include "HashMap.h"
#include "MaterialProperty.h"

class Material;
class MaterialData;

/**
 * Stores the stateful material properties computed by materials.
 *
 * Thread-safe
 */
class MaterialPropertyStorage
{
public:
  MaterialPropertyStorage();
  virtual ~MaterialPropertyStorage();

  void releaseProperties();

  /**
   * Initialize stateful material properties
   * @param material_data MaterilData object used for computing the data
   * @param mats Materials that will compute the initial values
   * @param n_qpoints Number of quadrature points
   * @param elem Element we are on
   * @param side Side of the element 'elem' (0 for volumetric material properties)
   */
  void initStatefulProps(MaterialData & material_data, std::vector<Material *> & mats, unsigned int n_qpoints, const Elem & elem, unsigned int side = 0);

  /**
   * Shift the material properties in time.
   *
   * Old material properties become older, current material properties become old. Older material properties are
   * reused for computing current properties. This is called when solve succeeded.
   */
  void shift();

  /**
   * Swap (shallow copy) material properties in MaterialData and MaterialPropertyStorage
   * Thread safe
   * @param material_data MaterialData object to work with
   * @param elem Element id
   * @param side Side number (elemental material properties have this equal to zero)
   */
  void swap(MaterialData & material_data, const Elem & elem, unsigned int side);

  /**
   * Swap (shallow copy) material properties in MaterialPropertyStorage and MaterialDat
   * Thread safe
   * @param material_data MaterialData object to work with
   * @param elem Element id
   * @param side Side number (elemental material properties have this equal to zero)
   */
  void swapBack(MaterialData & material_data, const Elem & elem, unsigned int side);

  /**
   * @return a Boolean indicating whether stateful properties exist on this material
   */
  bool hasStatefulProperties() const { return _has_stateful_props; }

  /**
   * @return a Boolean indicating whether or not this material has older properties declared
   */
  bool hasOlderProperties() const { return _has_older_prop; }

  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > & props() { return *_props_elem; }
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > & propsOld() { return *_props_elem_old; }
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > & propsOlder() { return *_props_elem_older; }

  bool hasProperty(const std::string & prop_name) const;
  unsigned int addProperty(const std::string & prop_name);
  unsigned int addPropertyOld(const std::string & prop_name);
  unsigned int addPropertyOlder(const std::string & prop_name);

  std::set<unsigned int> & statefulProps() { return _stateful_props; }
  std::map<unsigned int, std::string> statefulPropNames() { return _prop_names; }

  unsigned int getPropertyId (const std::string & prop_name) const;

protected:
  // indexing: [element][side]->material_properties
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > * _props_elem;
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > * _props_elem_old;
  HashMap<unsigned int, HashMap<unsigned int, MaterialProperties> > * _props_elem_older;

  /// mapping from property name to property ID
  /// NOTE: this is static so the property numbering is global within the simulation (not just FEProblem - should be useful when we will use material properties from
  /// one FEPRoblem in another one - if we will ever do it)
  static std::map<std::string, unsigned int> _prop_ids;

  /**
   * Whether or not we have stateful properties.  This will get automatically
   * set to true if a stateful property is declared.
   */
  bool _has_stateful_props;

  /**
   * True if any material requires older properties to be computed.  This will get automatically
   * set to true if a older stateful property is declared.
   */
  bool _has_older_prop;

  /// mapping from property ID to property name
  std::map<unsigned int, std::string> _prop_names;
  /// list of property ids of stateful material properties
  std::set<unsigned int> _stateful_props;

  unsigned int addPropertyId (const std::string & prop_name);
};


#endif /* MATERIALPROPERTYSTORAGE_H */
