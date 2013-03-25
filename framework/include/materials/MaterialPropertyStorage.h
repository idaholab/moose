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

#include "Moose.h"
#include "MaterialProperty.h"
#include "HashMap.h"

//libMesh
#include "libmesh/elem.h"
#include "libmesh/quadrature.h"

#include <vector>
#include <map>
#include <string>

class Material;
class MaterialData;
class QpMap;

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
   * Creates storage for newly created elements from mesh Adaptivity.  Also, copies values from the parent qps to the new children.
   *
   * Note - call this on the MaterialPropertyStorage object for the _children_ that you want to project to.  ie, if you are trying
   * to project to the sides of the children, then call this on the boundary MaterialPropertyStorage.  Pass in the parent
   * MaterialPropertyStorage you are projecting _from_.  ie the volume one if you are projecting to "internal" child element faces.
   *
   * There are 3 cases here:
   *
   * 1. Volume to volume (parent_side = -1, child = -1, child_side = -1)
   *    Call on volume MaterialPropertyStorage and pass volume MaterialPropertyStorage for parent_material_props
   *
   * 2. Parent side to child side (parent_side = 0+, child = -1, child_side = 0+) where parent_side == child_side
   *    Call on boundary MaterialPropertyStorage and pass boundary MaterialPropertyStorage for parent_material_props
   *
   * 3. Child side to parent volume (parent_side = -1, child = 0+, child_side = 0+)
   *    Call on boundary MaterialPropertyStorage and pass volume MaterialPropertyStorage for parent_material_props
   *
   * @param qrule The current quadrature rule
   * @param qrule The current face qrule
   * @param parent_material_props The place to pull parent material property values from
   * @param child_material_data MaterialData object used for computing the data
   * @param mats Materials that will compute the initial values
   * @param n_qpoints Number of quadrature points
   * @param elem The parent element that was just refined
   * @param
   */
  void prolongStatefulProps(const std::vector<std::vector<QpMap> > & refinement_map,
                            QBase & qrule,
                            QBase & qrule_face,
                            MaterialPropertyStorage & parent_material_props,
                            MaterialData & child_material_data,
                            const Elem & elem,
                            const int input_parent_side,
                            const int input_child,
                            const int input_child_side);

  /**
   * Creates storage for newly created elements from mesh Adaptivity.  Also, copies values from the children to the parent.
   *
   * @param qrule The current quadrature rule
   * @param qrule The current face qrule
   * @param material_data MaterilData object used for computing the data
   * @param mats Materials that will compute the initial values
   * @param n_qpoints Number of quadrature points
   * @param elem The parent element that was just refined
   * @param side Side of the element 'elem' (0 for volumetric material properties)
   */
  void restrictStatefulProps(const std::vector<std::pair<unsigned int, QpMap> > & coarsening_map,
                             std::vector<const Elem *> & coarsened_element_children,
                             QBase & qrule,
                             QBase & qrule_face,
                             MaterialData & material_data,
                             const Elem & elem,
                             int input_side=-1);

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

  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & props() { return *_props_elem; }
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOld() { return *_props_elem_old; }
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > & propsOlder() { return *_props_elem_older; }

  bool hasProperty(const std::string & prop_name) const;
  unsigned int addProperty(const std::string & prop_name);
  unsigned int addPropertyOld(const std::string & prop_name);
  unsigned int addPropertyOlder(const std::string & prop_name);

  std::vector<unsigned int> & statefulProps() { return _stateful_prop_id_to_prop_id; }
  std::map<unsigned int, std::string> statefulPropNames() { return _prop_names; }

  unsigned int getPropertyId (const std::string & prop_name);

protected:
  // indexing: [element][side]->material_properties
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > * _props_elem;
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > * _props_elem_old;
  HashMap<const Elem *, HashMap<unsigned int, MaterialProperties> > * _props_elem_older;

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
  /// the vector of stateful property ids (the vector index is the map to stateful prop_id)
  std::vector<unsigned int> _stateful_prop_id_to_prop_id;

  unsigned int addPropertyId (const std::string & prop_name);
};


#endif /* MATERIALPROPERTYSTORAGE_H */
