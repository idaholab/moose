//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

#include "libmesh/enum_elem_type.h"
#include "libmesh/enum_elem_quality.h"

namespace libMesh
{
class EquationSystems;
class System;
class MeshBase;
}

/**
 * Report mesh information, such as the number of elements, nodes, and degrees of freedom.
 */
class MeshInfo : public GeneralReporter
{
public:
  static InputParameters validParams();

  MeshInfo(const InputParameters & parameters);

  /**
   * Type of a bound for DomainQuality (min or max).
   */
  enum BoundType : int
  {
    MIN = 0,
    MAX = 1
  };

  /**
   * Key for a libMesh::ElemQuality quality type with a bound (min or max)
   * to be used over an entire domain (sideset or subdomain).
   */
  struct DomainQuality : public std::pair<libMesh::ElemQuality, BoundType>
  {
    DomainQuality() = default;
    DomainQuality(const libMesh::ElemQuality elem_quality, const BoundType bound_type);

    /// Get the libMesh::ElemQuality entry
    ///@{
    libMesh::ElemQuality & elemQuality() { return this->first; }
    const libMesh::ElemQuality & elemQuality() const { return this->first; }
    ///@}

    /// Get the type of the bound (min or max)
    ///@{
    BoundType & boundType() { return this->second; }
    const BoundType & boundType() const { return this->second; }
    ///@}

    /// Get the name for this quality ([min,max]_[quality])
    std::string itemName() const;
    /// Update the value in the quality map
    void updateValue(std::map<DomainQuality, Real> & quality_map, const Real value) const;
  };

  /// The qualities (ElemQuality and bound type) to include for
  /// domain quantities (subdomains and sidesets)
  static const std::vector<DomainQuality> domain_qualities;
  /// The qualities to include for elems
  static const std::vector<libMesh::ElemQuality> elem_qualities;

  /**
   * Base struct for information for a single entry (elem, sideset, subdomain).
   *
   * @tparam IDType The type of the ID.
   */
  template <typename IDType>
  struct InfoBase
  {
    using id_type = IDType;

    InfoBase() = default;
    InfoBase(const IDType id) : id(id) {}

    /// ID
    IDType id;
  };

  /**
   * Struct for element-containing entries (elem, sideset, subdomain).
   *
   * @tparam IDType The type of the ID.
   */
  template <typename IDType>
  struct ElemContainingInfo : public InfoBase<IDType>
  {
    ElemContainingInfo() = default;
    ElemContainingInfo(const IDType id) : InfoBase<IDType>(id) {}

    // Bounding box
    BoundingBox bounding_box;
    // Total volume
    Real volume = 0;
  };

  /**
   * Structure for a single domain entry (sideset or subdomain).
   *
   * @tparam IDType The type of the ID.
   * @tparam ElemsType The type for the elements entry (different for sidesets and subdomains).
   */
  template <typename IDType, typename ElemsType>
  struct DomainInfo : public ElemContainingInfo<IDType>
  {
    DomainInfo() = default;
    DomainInfo(const IDType id) : ElemContainingInfo<IDType>(id) {}

    /// Name
    std::string name;
    /// Bounded (min, max) element qualities
    std::map<DomainQuality, Real> qualities;
    /// Elements in the domain
    std::vector<ElemsType> elems;
    /// Type(s) of elements in the domain
    std::set<libMesh::ElemType> elem_types;
    /// Minimum volume
    Real min_volume = std::numeric_limits<Real>::max();
    /// Maximum volume
    Real max_volume = 0;
    /// Number of elements
    std::size_t num_elems = 0;
    /// Processors the elements are on
    std::set<processor_id_type> processor_ids;
  };
  /// Information storage for a single sideset
  using SidesetInfo = DomainInfo<BoundaryID, std::pair<dof_id_type, unsigned int>>;
  /// Information storage for a single subdomain
  using SubdomainInfo = DomainInfo<SubdomainID, dof_id_type>;

  /**
   * Structure for a single elem entry.
   */
  struct ElemInfo : public ElemContainingInfo<dof_id_type>
  {
    ElemInfo() = default;
    ElemInfo(const dof_id_type id, const SubdomainID subdomain_id)
      : ElemContainingInfo<dof_id_type>(id), subdomain_id(subdomain_id)
    {
    }

    /// The element's subdomain ID
    SubdomainID subdomain_id;
    /// Evaluated element qualities qualities
    std::vector<std::pair<libMesh::ElemQuality, Real>> qualities;
    /// The dimensionality of the element
    unsigned short dim;
    /// The element mapping type
    libMesh::ElemMappingType elem_mapping_type;
    /// The element type
    libMesh::ElemType elem_type;
    /// The maximum vertex seperation for the element
    Real hmax;
    /// The minimum vertex seperation for the element
    Real hmin;
    /// The ID of each neighbor (indexed by side)
    std::vector<dof_id_type> neighbor_ids;
    /// The IDs of each node
    std::vector<dof_id_type> node_ids;
    /// The number of sides on the element
    unsigned int num_sides;
    /// The element's points
    std::vector<Point> points;
    /// The processor ID the element is on
    processor_id_type processor_id;
    /// Unique ID of the element
    unique_id_type unique_id;
  };

  /**
   * Base struct for defining which items are to be output for a domain that
   * contains elements (elems, sidesets, subdomains).
   */
  struct ElemContainingInfoItems
  {
    ElemContainingInfoItems() = default;
    ElemContainingInfoItems(const MultiMooseEnum & items);

    /// Markers for whether or not a single item should be output
    ///@{
    bool bounding_box = false;
    bool volume = false;
    ///@}
  };

  /**
   * Defines which items are to be output for a domain (sideset or subdomain).
   */
  struct DomainInfoItems : public ElemContainingInfoItems
  {
    DomainInfoItems() = default;
    DomainInfoItems(const MultiMooseEnum & items, const MultiMooseEnum & qualities);

    /// Markers for whether or not a single item should be output
    ///@{
    bool elems = false;
    bool elem_types = false;
    bool max_volume = false;
    bool min_volume = false;
    bool num_elems = false;
    bool processor_ids = false;
    ///@}

    /// Bounded (min, max) element qualities that should be output
    std::vector<DomainQuality> qualities;
  };

  struct ElemInfoItems : public ElemContainingInfoItems
  {
    ElemInfoItems() = default;
    ElemInfoItems(const MultiMooseEnum & items, const MultiMooseEnum & qualities);

    /// Markers for whether or not a single item should be output
    ///@{
    bool dim = false;
    bool elem_mapping_type = false;
    bool elem_type = false;
    bool hmax = false;
    bool hmin = false;
    bool neighbor_ids = false;
    bool node_ids = false;
    bool num_sides = false;
    bool points = false;
    bool processor_id = false;
    bool unique_id = false;
    ///@}
    /// Element qualities that should be output
    std::vector<libMesh::ElemQuality> qualities;
  };

  /**
   * Struct that defines a domain (sideset or subdomain) map (id -> entities) and
   * the items that should be output about that domain.
   *
   * @tparam InfoType The type of the information storage for each entity.
   * @tparam ItemsType The type of the items storage that define what to output.
   */
  template <class InfoType, class ItemsType>
  struct InfoMap
  {
    using id_type = typename InfoType::id_type;
    using info_type = InfoType;
    using map_type = std::map<id_type, InfoType>;

    InfoMap() = default;
    InfoMap(const ItemsType & items) : items(items) {}

    /// The underlying data
    map_type map;
    /// Which items are to be output
    ItemsType items;
  };
  /// Type for reporter information for an element
  using ElemInfoMap = InfoMap<ElemInfo, ElemInfoItems>;
  /// Type for reporter information for a sideset
  using SidesetInfoMap = InfoMap<SidesetInfo, DomainInfoItems>;
  /// Type for reporter information for a subdomain
  using SubdomainInfoMap = InfoMap<SubdomainInfo, DomainInfoItems>;

  /**
   * Struct that contains all of the reporter data for a domain type (sideset or subdomain)
   * and the items that should be output for that domain.
   *
   * @tparam InfoMapType The type of the storage map (id -> information).
   * @tparam ItemsType The type of the items storage that define what to output.

   */
  template <class InfoMapType, class ItemsType>
  struct CombinedInfos
  {
    using info_map_type = InfoMapType;
    using id_type = typename InfoMapType::id_type;
    using info_type = typename InfoMapType::info_type;
    using map_type = typename info_map_type::map_type;
    using items_type = ItemsType;

    CombinedInfos(info_map_type * local, info_map_type * global, const ItemsType & items);

    /// Local information
    InfoMapType * const local;
    /// Global information
    InfoMapType * const global;
    /// Which entries are to be output
    const ItemsType items;
  };
  /// Complete information storage (local, global, items) for sidesets
  using ElemInfos = CombinedInfos<ElemInfoMap, ElemInfoItems>;
  /// Complete information storage (local, global, items) for sidesets
  using SidesetInfos = CombinedInfos<SidesetInfoMap, DomainInfoItems>;
  /// Complete information storage (local, global, items) for subdomains
  using SubdomainInfos = CombinedInfos<SubdomainInfoMap, DomainInfoItems>;

  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

  static bool hasItem(const std::string & name, const MultiMooseEnum & items);

protected:
  /// The requested items
  const MultiMooseEnum & _items;
  /// The requested elem items
  const MultiMooseEnum & _elem_items;
  /// The requested sideset items
  const MultiMooseEnum & _sideset_items;
  /// The requested subdomain items
  const MultiMooseEnum & _subdomain_items;
  /// The requested sideset qualities
  const MultiMooseEnum & _elem_qualities;
  /// The requested sideset qualities
  const MultiMooseEnum & _sideset_qualities;
  /// The requested subdomain qualities
  const MultiMooseEnum & _subdomain_qualities;

  /**
   * Helper to perform optional declaration based on if the item
   * is requested or all are requested. If neither, returns nullptr.
   */
  template <typename T, typename... Args>
  T * declareHelper(const std::string & name, const ReporterMode mode, Args &&... args);

private:
  /// Initializer for SidesetInfos and SubdomainInfos
  template <class CombinedInfosType>
  CombinedInfosType initCombinedInfos(const std::string & name,
                                      const MultiMooseEnum & items,
                                      const MultiMooseEnum & qualities);

  /// Add elem information if requested
  void possiblyAddElemInfo();

  /// Add domain information (sidesets and subdomains) if requested
  template <class CombinedInfosType>
  void possiblyAddDomainInfo(CombinedInfosType & infos);

  /**
   * Reporter values to declare.
   *
   * Will be nullptr if not requested.
   */
  ///@{
  unsigned int * const _num_dofs;
  unsigned int * const _num_dofs_nl;
  unsigned int * const _num_dofs_aux;
  unsigned int * const _num_dofs_constrained;
  unsigned int * const _num_elem;
  unsigned int * const _num_node;
  unsigned int * const _num_local_dofs;
  unsigned int * const _num_local_dofs_nl;
  unsigned int * const _num_local_dofs_aux;
  unsigned int * const _num_local_elem;
  unsigned int * const _num_local_node;
  ///@}

  /// Combined element information (reporter values and requested items)
  ElemInfos _elem_infos;
  /// Combined sideset information (reporter values and requested items)
  SidesetInfos _sideset_infos;
  /// Combined subdomain information (reporter values and requested items)
  SubdomainInfos _subdomain_infos;

  const libMesh::EquationSystems & _equation_systems;
  const libMesh::System & _nonlinear_system;
  const libMesh::System & _aux_system;
  const libMesh::MeshBase & _mesh;
};

template <typename T, typename... Args>
T *
MeshInfo::declareHelper(const std::string & name, const ReporterMode mode, Args &&... args)
{
  return hasItem(name, _items) ? &declareValueByName<T>(name, mode, args...) : nullptr;
}

/**
 * JSON serialization for info maps
 */
///@{
void to_json(nlohmann::json &, const MeshInfo::ElemInfoMap &);
void to_json(nlohmann::json &, const MeshInfo::SidesetInfoMap &);
void to_json(nlohmann::json &, const MeshInfo::SubdomainInfoMap &);
///@}

/**
 * Data store and load for DomainQuality
 */
///@{
void dataStore(std::ostream &, MeshInfo::DomainQuality &, void *);
void dataLoad(std::istream &, MeshInfo::DomainQuality &, void *);
///@}

/**
 * Data store and load for node, elem, sideset, subdomain info entries
 */
///@{
void dataStore(std::ostream &, MeshInfo::ElemInfo &, void *);
void dataStore(std::ostream &, MeshInfo::SidesetInfo &, void *);
void dataStore(std::ostream &, MeshInfo::SubdomainInfo &, void *);
void dataLoad(std::istream &, MeshInfo::ElemInfo &, void *);
void dataLoad(std::istream &, MeshInfo::SidesetInfo &, void *);
void dataLoad(std::istream &, MeshInfo::SubdomainInfo &, void *);
///@}

/**
 * Data store and load for info maps
 */
///@{
void dataStore(std::ostream &, MeshInfo::ElemInfoMap &, void *);
void dataStore(std::ostream &, MeshInfo::SidesetInfoMap &, void *);
void dataStore(std::ostream &, MeshInfo::SubdomainInfoMap &, void *);
void dataLoad(std::istream &, MeshInfo::ElemInfoMap &, void *);
void dataLoad(std::istream &, MeshInfo::SidesetInfoMap &, void *);
void dataLoad(std::istream &, MeshInfo::SubdomainInfoMap &, void *);
///@}
