//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "HashMap.h"
#include "DataIO.h"
#include "MaterialProperty.h"

// Forward declarations
class MaterialBase;
class MaterialData;
class QpMap;

// libMesh forward declarations
namespace libMesh
{
class QBase;
class Elem;
}

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

  /**
   * Creates storage for newly created elements from mesh Adaptivity.  Also, copies values from the
   * parent qps to the new children.
   *
   * Note - call this on the MaterialPropertyStorage object for the _children_ that you want to
   * project to.  ie, if you are trying
   * to project to the sides of the children, then call this on the boundary
   * MaterialPropertyStorage.  Pass in the parent
   * MaterialPropertyStorage you are projecting _from_.  ie the volume one if you are projecting to
   * "internal" child element faces.
   *
   * There are 3 cases here:
   *
   * 1. Volume to volume (parent_side = -1, child = -1, child_side = -1)
   *    Call on volume MaterialPropertyStorage and pass volume MaterialPropertyStorage for
   * parent_material_props
   *
   * 2. Parent side to child side (parent_side = 0+, child = -1, child_side = 0+) where parent_side
   * == child_side
   *    Call on boundary MaterialPropertyStorage and pass boundary MaterialPropertyStorage for
   * parent_material_props
   *
   * 3. Child side to parent volume (parent_side = -1, child = 0+, child_side = 0+)
   *    Call on boundary MaterialPropertyStorage and pass volume MaterialPropertyStorage for
   * parent_material_props
   *
   * @param pid - processor id of children to prolong to
   * @param refinement_map - 2D array of QpMap objects
   * @param qrule The current quadrature rule
   * @param qrule_face The current face qrule
   * @param parent_material_props The place to pull parent material property values from
   * @param child_material_data MaterialData object used for computing the data
   * @param elem The parent element that was just refined
   * @param input_parent_side - the side of the parent for which material properties are prolonged
   * @param input_child - the number of the child
   * @param input_child_side - the side on the child where material properties will be prolonged
   */
  void prolongStatefulProps(processor_id_type pid,
                            const std::vector<std::vector<QpMap>> & refinement_map,
                            const QBase & qrule,
                            const QBase & qrule_face,
                            MaterialPropertyStorage & parent_material_props,
                            MaterialData & child_material_data,
                            const Elem & elem,
                            const int input_parent_side,
                            const int input_child,
                            const int input_child_side);

  /**
   * Creates storage for newly created elements from mesh Adaptivity.  Also, copies values from the
   * children to the parent.
   *
   * @param coarsening_map - map from unsigned ints to QpMap's
   * @param coarsened_element_children - a pointer to a vector of coarsened element children
   * @param qrule The current quadrature rule
   * @param qrule_face The current face qrule
   * @param material_data MaterialData object used for computing the data
   * @param elem The parent element that was just refined
   * @param input_side Side of the element 'elem' (0 for volumetric material properties)
   */
  void restrictStatefulProps(const std::vector<std::pair<unsigned int, QpMap>> & coarsening_map,
                             const std::vector<const Elem *> & coarsened_element_children,
                             const QBase & qrule,
                             const QBase & qrule_face,
                             MaterialData & material_data,
                             const Elem & elem,
                             int input_side = -1);

  /**
   * Initialize stateful material properties
   * @param material_data MaterilData object used for computing the data
   * @param mats Materials that will compute the initial values
   * @param n_qpoints Number of quadrature points
   * @param elem Element we are on
   * @param side Side of the element 'elem' (0 for volumetric material properties)
   */
  void initStatefulProps(MaterialData & material_data,
                         const std::vector<std::shared_ptr<MaterialBase>> & mats,
                         unsigned int n_qpoints,
                         const Elem & elem,
                         unsigned int side = 0);

  /**
   * Shift the material properties in time.
   *
   * Old material properties become older, current material properties become old. Older material
   * properties are
   * reused for computing current properties. This is called when solve succeeded.
   */
  void shift();

  /**
   * Copy material properties from elem_from to elem_to. Thread safe.
   *
   * WARNING: This is not capable of copying material data to/from elements on other processors.
   *          It only works if both elem_to and elem_from are both on the local processor.
   *          We can't currently check to ensure that they're on processor here because this isn't a
   * ParallelObject.
   *
   * @param material_data MaterialData object to work with
   * @param elem_to Element to copy data to
   * @param elem_from Element to copy data from
   * @param side Side number (elemental material properties have this equal to zero)
   * @param n_qpoints number of quadrature points to work with
   */
  void copy(MaterialData & material_data,
            const Elem & elem_to,
            const Elem & elem_from,
            unsigned int side,
            unsigned int n_qpoints);

  /**
   * Copy material properties from elem_from to elem_to.
   * Similar to the other method but using pointers to elements instead of references.
   *
   * @param material_data MaterialData object to work with
   * @param elem_to Pointer to the element to copy data to
   * @param elem_from Pointer to the element to copy data from
   * @param side Side number (elemental material properties have this equal to zero)
   * @param n_qpoints number of quadrature points to work with
   */
  void copy(MaterialData & material_data,
            const Elem * elem_to,
            const Elem * elem_from,
            unsigned int side,
            unsigned int n_qpoints);

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

  /**
   * Accessible type of the stored material property data.
   *
   * This probably should have been returned as a proxy class; only
   * access it via foo[elem][side] and maybe we'll be able to refactor
   * it in the future without breaking your code.
   */
  typedef HashMap<const Elem *, HashMap<unsigned int, MaterialProperties>> PropsType;

  ///@{
  /**
   * Access methods to the stored material property data.
   */
  const PropsType & props() const { return *_props_elem; }
  const PropsType & propsOld() const { return *_props_elem_old; }
  const PropsType & propsOlder() const { return *_props_elem_older; }
  const MaterialProperties & props(const Elem * elem, unsigned int side) const;
  const MaterialProperties & propsOld(const Elem * elem, unsigned int side) const;
  const MaterialProperties & propsOlder(const Elem * elem, unsigned int side) const;
  MaterialProperties & setProps(const Elem * elem, unsigned int side);
  MaterialProperties & setPropsOld(const Elem * elem, unsigned int side);
  MaterialProperties & setPropsOlder(const Elem * elem, unsigned int side);
  ///@}

  bool hasProperty(const std::string & prop_name) const;

  /// The addProperty functions are idempotent - calling multiple times with
  /// the same name will provide the same id and works fine.
  unsigned int addProperty(const std::string & prop_name);
  unsigned int addPropertyOld(const std::string & prop_name);
  unsigned int addPropertyOlder(const std::string & prop_name);

  std::vector<unsigned int> & statefulProps() { return _stateful_prop_id_to_prop_id; }
  const std::vector<unsigned int> & statefulProps() const { return _stateful_prop_id_to_prop_id; }
  const std::map<unsigned int, std::string> statefulPropNames() const { return _prop_names; }

  /// Returns the property ID for the given prop_name, adding the property and
  /// creating a new ID if it hasn't already been created.
  unsigned int getPropertyId(const std::string & prop_name);

  unsigned int retrievePropertyId(const std::string & prop_name) const;

  bool isStatefulProp(const std::string & prop_name) const
  {
    return _prop_names.count(retrievePropertyId(prop_name)) > 0;
  }

  /**
   * Remove the property storage and element pointer from internal data structures
   * Use this when elements are deleted so we don't end up with invalid elem pointers (for e.g.
   * stateful properties) hanging around in our data structures
   */
  void eraseProperty(const Elem * elem);

  static const std::map<std::string, unsigned int> & propIDs() { return _prop_ids; }

protected:
  /// Release all internal data structures
  void releaseProperties();

  /// Internal property storage release helper
  void releasePropertyMap(HashMap<unsigned int, MaterialProperties> & inner_map);

  // indexing: [element][side]->material_properties
  std::unique_ptr<PropsType> _props_elem;
  std::unique_ptr<PropsType> _props_elem_old;
  std::unique_ptr<PropsType> _props_elem_older;

  /// mapping from property name to property ID
  /// NOTE: this is static so the property numbering is global within the simulation (not just FEProblemBase - should be useful when we will use material properties from
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

  void sizeProps(MaterialProperties & mp, unsigned int size);

private:
  /// Initializes hashmap entries for element and side to proper qpoint and
  /// property count sizes.
  void initProps(MaterialData & material_data,
                 const Elem * elem,
                 unsigned int side,
                 unsigned int n_qpoints);

  /// Initializes just one hashmap's entries
  void initProps(MaterialData & material_data,
                 PropsType & mat_props_map,
                 const Elem * elem,
                 unsigned int side,
                 unsigned int n_qpoints);

  // You'd think a private mutex would work here, so I'll leave this
  // in the namespace for when that happens, but CI thinks that can
  // make us crash, so I'll just initialize this to Threads::spin_mtx
  libMesh::Threads::spin_mutex & _spin_mtx;

  // Need to be able to eraseProperty from here
  friend class ProjectMaterialProperties;

  // Need to be able to initProps from here
  friend class RedistributeProperties;

  // Need non-const props from here
  friend void dataLoad<MaterialPropertyStorage>(std::istream &, MaterialPropertyStorage &, void *);
  friend void dataStore<MaterialPropertyStorage>(std::ostream &, MaterialPropertyStorage &, void *);
};

inline const MaterialProperties &
MaterialPropertyStorage::props(const Elem * elem, unsigned int side) const
{
  mooseAssert(props().contains(elem), "Trying to read properties on an element that lacks them");
  mooseAssert(props().find(elem)->second.contains(side),
              "Trying to read properties on an element side that lacks them");
  return props().find(elem)->second.find(side)->second;
}

inline const MaterialProperties &
MaterialPropertyStorage::propsOld(const Elem * elem, unsigned int side) const
{
  mooseAssert(propsOld().contains(elem),
              "Trying to read old properties on an element that lacks them");
  mooseAssert(propsOld().find(elem)->second.contains(side),
              "Trying to read old properties on an element side that lacks them");
  return propsOld().find(elem)->second.find(side)->second;
}

inline const MaterialProperties &
MaterialPropertyStorage::propsOlder(const Elem * elem, unsigned int side) const
{
  mooseAssert(propsOlder().contains(elem),
              "Trying to read older properties on an element that lacks them");
  mooseAssert(propsOlder().find(elem)->second.contains(side),
              "Trying to read older properties on an element side that lacks them");
  return propsOlder().find(elem)->second.find(side)->second;
}

inline MaterialProperties &
MaterialPropertyStorage::setProps(const Elem * elem, unsigned int side)
{
  // Many problems rely on reinitMaterials also being the first
  // init, and I'm not sure I can clean that up to reallow these
  // assertions without also hurting performance on subsequent
  // iterations.
  // libmesh_assert(_props_elem->contains(elem));
  // libmesh_assert((*_props_elem)[elem].contains(side));
  return (*_props_elem)[elem][side];
}

inline MaterialProperties &
MaterialPropertyStorage::setPropsOld(const Elem * elem, unsigned int side)
{
  // libmesh_assert(_props_elem_old->contains(elem));
  // libmesh_assert((*_props_elem_old)[elem].contains(side));
  return (*_props_elem_old)[elem][side];
}

inline MaterialProperties &
MaterialPropertyStorage::setPropsOlder(const Elem * elem, unsigned int side)
{
  // libmesh_assert(_props_elem_older->contains(elem));
  // libmesh_assert((*_props_elem_older)[elem].contains(side));
  return (*_props_elem_older)[elem][side];
}

template <>
inline void
dataStore(std::ostream & stream, MaterialPropertyStorage & storage, void * context)
{
  dataStore(stream, *storage._props_elem, context);
  dataStore(stream, *storage._props_elem_old, context);

  if (storage.hasOlderProperties())
    dataStore(stream, *storage._props_elem_older, context);
}

template <>
inline void
dataLoad(std::istream & stream, MaterialPropertyStorage & storage, void * context)
{
  dataLoad(stream, *storage._props_elem, context);
  dataLoad(stream, *storage._props_elem_old, context);

  if (storage.hasOlderProperties())
    dataLoad(stream, *storage._props_elem_older, context);
}
