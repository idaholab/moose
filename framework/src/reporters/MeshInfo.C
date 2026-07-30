//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshInfo.h"
#include "SubProblem.h"
#include "libmesh/system.h"
#include "libmesh/equation_systems.h"
#include "libmesh/parallel_sync.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/elem_side_builder.h"
#include "libmesh/enum_elem_quality.h"
#include "libmesh/enum_to_string.h"

registerMooseObject("MooseApp", MeshInfo);

/// Element qualities with a bound (min or max) to support on domains (sidesets, subdomains)
const std::vector<MeshInfo::DomainQuality> MeshInfo::domain_qualities{
    {libMesh::ElemQuality::MIN_ANGLE, MeshInfo::BoundType::MIN},
    {libMesh::ElemQuality::MAX_ANGLE, MeshInfo::BoundType::MAX},
    {libMesh::ElemQuality::JACOBIAN, MeshInfo::BoundType::MIN},
    {libMesh::ElemQuality::JACOBIAN, MeshInfo::BoundType::MAX}};
/// Element qualities to support on elems
const std::vector<libMesh::ElemQuality> MeshInfo::elem_qualities{libMesh::ElemQuality::MIN_ANGLE,
                                                                 libMesh::ElemQuality::MAX_ANGLE,
                                                                 libMesh::ElemQuality::JACOBIAN};

std::string
elemQualityToString(const libMesh::ElemQuality eq)
{
  return MooseUtils::toLower(libMesh::Utility::enum_to_string(eq));
}

InputParameters
MeshInfo::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Report mesh information, such as the number of elements, nodes, and degrees of freedom.");

  MultiMooseEnum items("elems num_dofs num_dofs_nonlinear num_dofs_auxiliary num_dofs_constrained "
                       "num_elements num_nodes num_local_dofs num_local_dofs_nonlinear "
                       "num_local_dofs_auxiliary num_local_elements num_local_nodes local_elems "
                       "local_sidesets local_subdomains sidesets subdomains");
  params.addParam<MultiMooseEnum>(
      "items",
      items,
      "The iteration information to output; if not provided, everything will be output.");

  // Elem parameters
  {
    {
      MultiMooseEnum items("all bounding_box dim elem_mapping_type elem_type hmax hmin "
                           "neighbor_ids node_ids num_sides points processor_id unique_id volume");
      params.addParam<MultiMooseEnum>(
          "elem_items", items, "Items to include when outputting elem information");
    }

    {
      std::vector<std::string> elem_qualities;
      elem_qualities.reserve(MeshInfo::elem_qualities.size());
      std::transform(MeshInfo::elem_qualities.begin(),
                     MeshInfo::elem_qualities.end(),
                     std::back_inserter(elem_qualities),
                     [](const auto v) { return elemQualityToString(v); });
      std::sort(elem_qualities.begin(), elem_qualities.end());
      MultiMooseEnum items("all " + MooseUtils::stringJoin(elem_qualities, " "));
      params.addParam<MultiMooseEnum>(
          "elem_qualities", items, "Element qualities to include when outputting elem information");
    }
  }

  // Domain parameters (sidesets, subdomains)
  {
    static const std::array<std::string, 2> domain_names{"sideset", "subdomain"};

    // [sideset,subdomain]_items
    {
      MultiMooseEnum items(
          "all bounding_box elems elem_types min_volume max_volume num_elems processor_ids volume");
      for (const auto & name : domain_names)
        params.addParam<MultiMooseEnum>(
            name + "_items", items, "Items to include when outputting " + name + " information");
    }
    // [sideset,subdomain]_qualities
    {
      // Convert each entry in MeshInfo::domain_qualities to a human string, like:
      // {libMesh::ElemQuality::MIN_ANGLE, MeshInfo::BoundType::MIN} -> "min_min_angle"
      std::vector<std::string> domain_qualities;
      domain_qualities.reserve(MeshInfo::domain_qualities.size());
      std::transform(MeshInfo::domain_qualities.begin(),
                     MeshInfo::domain_qualities.end(),
                     std::back_inserter(domain_qualities),
                     [](const auto & v) { return v.itemName(); });
      std::sort(domain_qualities.begin(), domain_qualities.end());

      MultiMooseEnum items("all " + MooseUtils::stringJoin(domain_qualities, " "));
      for (const auto & name : domain_names)
        params.addParam<MultiMooseEnum>(name + "_qualities",
                                        items,
                                        "Element qualities to include when outputting " + name +
                                            " information");
    }
  }

  return params;
}

MeshInfo::MeshInfo(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _items(getParam<MultiMooseEnum>("items")),
    _elem_items(getParam<MultiMooseEnum>("elem_items")),
    _sideset_items(getParam<MultiMooseEnum>("sideset_items")),
    _subdomain_items(getParam<MultiMooseEnum>("subdomain_items")),
    _elem_qualities(getParam<MultiMooseEnum>("elem_qualities")),
    _sideset_qualities(getParam<MultiMooseEnum>("sideset_qualities")),
    _subdomain_qualities(getParam<MultiMooseEnum>("subdomain_qualities")),

    _num_dofs(declareHelper<unsigned int>("num_dofs", REPORTER_MODE_REPLICATED)),
    _num_dofs_nl(declareHelper<unsigned int>("num_dofs_nonlinear", REPORTER_MODE_REPLICATED)),
    _num_dofs_aux(declareHelper<unsigned int>("num_dofs_auxiliary", REPORTER_MODE_REPLICATED)),
    _num_dofs_constrained(
        declareHelper<unsigned int>("num_dofs_constrained", REPORTER_MODE_REPLICATED)),
    _num_elem(declareHelper<unsigned int>("num_elements", REPORTER_MODE_REPLICATED)),
    _num_node(declareHelper<unsigned int>("num_nodes", REPORTER_MODE_REPLICATED)),
    _num_local_dofs(declareHelper<unsigned int>("num_local_dofs", REPORTER_MODE_DISTRIBUTED)),
    _num_local_dofs_nl(
        declareHelper<unsigned int>("num_local_dofs_nonlinear", REPORTER_MODE_DISTRIBUTED)),
    _num_local_dofs_aux(
        declareHelper<unsigned int>("num_local_dofs_auxiliary", REPORTER_MODE_DISTRIBUTED)),
    _num_local_elem(declareHelper<unsigned int>("num_local_elements", REPORTER_MODE_DISTRIBUTED)),
    _num_local_node(declareHelper<unsigned int>("num_local_nodes", REPORTER_MODE_DISTRIBUTED)),

    _elem_infos(initCombinedInfos<ElemInfos>("elems", _elem_items, _elem_qualities)),
    _sideset_infos(initCombinedInfos<SidesetInfos>("sidesets", _sideset_items, _sideset_qualities)),
    _subdomain_infos(
        initCombinedInfos<SubdomainInfos>("subdomains", _subdomain_items, _subdomain_qualities)),

    _equation_systems(_fe_problem.es()),
    _nonlinear_system(_fe_problem.es().get_system("nl0")),
    _aux_system(_fe_problem.es().get_system("aux0")),
    _mesh(_fe_problem.mesh().getMesh())
{
  if (_elem_items.isValid() && !hasItem("elems", _items) && !hasItem("local_elems", _items))
    paramError("elem_items", "Should not be provided without an elems item enabled");
  if (_elem_qualities.isValid() && !hasItem("elems", _items) && !hasItem("local_elems", _items))
    paramError("elem_qualities", "Should not be provided without an elems item enabled");
  if (_sideset_items.isValid() && !hasItem("sidesets", _items) &&
      !hasItem("local_sidesets", _items))
    paramError("sideset_items", "Should not be provided without a sidesets item enabled");
  if (_sideset_qualities.isValid() && !hasItem("sidesets", _items) &&
      !hasItem("local_sidesets", _items))
    paramError("sideset_qualities", "Should not be provided without a sidesets item enabled");
  if (_subdomain_items.isValid() && !hasItem("subdomains", _items) &&
      !hasItem("local_subdomains", _items))
    paramError("subdomain_items", "Should not be provided without a subdomains item enabled");
  if (_subdomain_qualities.isValid() && !hasItem("subdomains", _items) &&
      !hasItem("local_subdomains", _items))
    paramError("subdomain_qualities", "Should not be provided without a subdomains item enabled");
}

MeshInfo::DomainQuality::DomainQuality(const libMesh::ElemQuality elem_quality,
                                       const BoundType bound_type)
  : std::pair<libMesh::ElemQuality, BoundType>(elem_quality, bound_type)
{
}

std::string
MeshInfo::DomainQuality::itemName() const
{
  return (boundType() == BoundType::MAX ? std::string("max") : std::string("min")) + "_" +
         elemQualityToString(elemQuality());
}

void
MeshInfo::DomainQuality::updateValue(std::map<DomainQuality, Real> & quality_map,
                                     const Real value) const
{
  static const Real default_max = 0;
  static const Real default_min = std::numeric_limits<Real>::max();
  const bool is_max = boundType() == BoundType::MAX;
  auto & entry = quality_map.try_emplace(*this, (is_max ? default_max : default_min)).first->second;
  if (is_max)
    entry = std::max(entry, value);
  else
    entry = std::min(entry, value);
}

MeshInfo::ElemContainingInfoItems::ElemContainingInfoItems(const MultiMooseEnum & items)
  : bounding_box(MeshInfo::hasItem("bounding_box", items)),
    volume(MeshInfo::hasItem("volume", items))
{
}

MeshInfo::DomainInfoItems::DomainInfoItems(const MultiMooseEnum & items,
                                           const MultiMooseEnum & qualities)
  : MeshInfo::ElemContainingInfoItems(items),
    elems(MeshInfo::hasItem("elems", items)),
    elem_types(MeshInfo::hasItem("elem_types", items)),
    max_volume(MeshInfo::hasItem("max_volume", items)),
    min_volume(MeshInfo::hasItem("min_volume", items)),
    num_elems(MeshInfo::hasItem("num_elems", items)),
    processor_ids(MeshInfo::hasItem("processor_ids", items))
{
  std::copy_if(MeshInfo::domain_qualities.begin(),
               MeshInfo::domain_qualities.end(),
               std::back_inserter(this->qualities),
               [&](const auto & v) { return MeshInfo::hasItem(v.itemName(), qualities); });
}

MeshInfo::ElemInfoItems::ElemInfoItems(const MultiMooseEnum & items,
                                       const MultiMooseEnum & qualities)
  : MeshInfo::ElemContainingInfoItems(items),
    dim(MeshInfo::hasItem("dim", items)),
    elem_mapping_type(MeshInfo::hasItem("elem_mapping_type", items)),
    elem_type(MeshInfo::hasItem("elem_type", items)),
    hmax(MeshInfo::hasItem("hmax", items)),
    hmin(MeshInfo::hasItem("hmin", items)),
    neighbor_ids(MeshInfo::hasItem("neighbor_ids", items)),
    node_ids(MeshInfo::hasItem("node_ids", items)),
    num_sides(MeshInfo::hasItem("num_sides", items)),
    points(MeshInfo::hasItem("points", items)),
    processor_id(MeshInfo::hasItem("processor_id", items)),
    unique_id(MeshInfo::hasItem("unique_id", items))
{
  std::copy_if(MeshInfo::elem_qualities.begin(),
               MeshInfo::elem_qualities.end(),
               std::back_inserter(this->qualities),
               [&](const auto v) { return MeshInfo::hasItem(elemQualityToString(v), qualities); });
}

template <typename InfoMapType, class InfoItemsType>
MeshInfo::CombinedInfos<InfoMapType, InfoItemsType>::CombinedInfos(InfoMapType * local,
                                                                   InfoMapType * global,
                                                                   const InfoItemsType & items)
  : local(local), global(global), items(items)
{
}

template <class CombinedInfosType>
CombinedInfosType
MeshInfo::initCombinedInfos(const std::string & name,
                            const MultiMooseEnum & items,
                            const MultiMooseEnum & qualities)
{
  using info_map_type = typename CombinedInfosType::info_map_type;
  const typename CombinedInfosType::items_type dii(items, qualities);
  return CombinedInfosType(
      declareHelper<info_map_type>("local_" + name, REPORTER_MODE_DISTRIBUTED, dii),
      declareHelper<info_map_type>(name, REPORTER_MODE_ROOT, dii),
      dii);
}

void
MeshInfo::possiblyAddElemInfo()
{
  auto local_ptr = _elem_infos.local;
  auto global_ptr = _elem_infos.global;

  // Nothing to do
  if (!local_ptr && !global_ptr)
    return;

  // Clear all entries first
  for (auto value_ptr : {local_ptr, global_ptr})
    if (value_ptr)
      value_ptr->map.clear();

  /// Helper for inserting an element into a map
  const auto map_insert = [](auto & map, const auto id, const auto subdomain_id) -> ElemInfo &
  {
    mooseAssert(!map.count(id), "Should not exist in map");
    return map
        .emplace(std::piecewise_construct,
                 std::forward_as_tuple(id),
                 std::forward_as_tuple(id, subdomain_id))
        .first->second;
  };

  const auto & items = _elem_infos.items;

#define set_simple(name, elem_function_name)                                                       \
  if (items.name)                                                                                  \
  entry.name = elem.elem_function_name()

  // Fill the local information
  std::map<dof_id_type, ElemInfo> local_info;
  for (const auto elem_ptr : *_fe_problem.mesh().getActiveLocalElementRange())
  {
    auto & elem = *elem_ptr;
    auto & entry = map_insert(local_info, elem.id(), elem.subdomain_id());

    entry.qualities.reserve(items.qualities.size());
    for (const auto quality : items.qualities)
      entry.qualities.emplace_back(quality, elem.quality(quality));

    set_simple(bounding_box, loose_bounding_box);
    set_simple(volume, volume);
    set_simple(dim, dim);
    set_simple(elem_mapping_type, mapping_type);
    set_simple(elem_type, type);
    set_simple(hmax, hmax);
    set_simple(hmin, hmin);
    if (items.neighbor_ids)
    {
      entry.neighbor_ids.reserve(elem.n_neighbors());
      for (const auto neighbor_ptr : elem.neighbor_ptr_range())
        entry.neighbor_ids.push_back(neighbor_ptr ? neighbor_ptr->id()
                                                  : libMesh::DofObject::invalid_id);
    }
    if (items.node_ids)
    {
      entry.node_ids.reserve(elem.n_nodes());
      for (const auto & node : elem.node_ref_range())
        entry.node_ids.push_back(node.id());
    }
    set_simple(num_sides, n_sides);
    if (items.points)
    {
      entry.points.reserve(elem.n_nodes());
      for (const auto & node : elem.node_ref_range())
        entry.points.emplace_back(node);
    }
    set_simple(processor_id, processor_id);
    if (items.unique_id)
      entry.unique_id =
          elem.valid_unique_id() ? elem.unique_id() : libMesh::DofObject::invalid_unique_id;
  }
#undef set_simple

  // For local, copy over everything we have. It's just local information.
  if (local_ptr)
    local_ptr->map = local_info;

  // For global entries, need to accumulate data
  if (global_ptr)
  {
    // initialize with ID and subdomain_id
    {
      std::vector<std::pair<dof_id_type, subdomain_id_type>> data;
      data.reserve(local_info.size());
      for (auto & [id, info] : local_info)
        data.emplace_back(id, info.subdomain_id);
      comm().gather(0, data);
      for (const auto & [id, subdomain_id] : data)
        map_insert(global_ptr->map, id, subdomain_id);
    }

    const auto gather = [&](const auto && get_value, const auto && set_value)
    {
      using T = std::remove_reference_t<std::invoke_result_t<decltype(get_value), ElemInfo &>>;
      std::vector<std::pair<dof_id_type, T>> data;
      data.reserve(local_info.size());
      for (auto & [id, info] : local_info)
        data.emplace_back(id, get_value(info));
      comm().gather(0, data);
      for (const auto & [id, value] : data)
        set_value(global_ptr->map.at(id), value);
    };

    // qualities
    if (items.qualities.size())
      gather(
          [](const auto & info)
          {
            std::vector<std::pair<std::underlying_type_t<libMesh::ElemQuality>, Real>> values;
            values.reserve(info.qualities.size());
            values.insert(values.end(), info.qualities.begin(), info.qualities.end());
            return values;
          },
          [](auto & info, const auto & value)
          {
            info.qualities.reserve(value.size());
            for (const auto & [elem_quality, quality_value] : value)
              info.qualities.emplace_back(static_cast<libMesh::ElemQuality>(elem_quality),
                                          quality_value);
          });
    // bounding_box
    if (items.bounding_box)
      gather([](const auto & info)
             { return static_cast<const std::pair<Point, Point> &>(info.bounding_box); },
             [](auto & info, const auto & value)
             { static_cast<std::pair<Point, Point> &>(info.bounding_box) = value; });

#define gather_simple(name)                                                                        \
  if (items.name)                                                                                  \
  gather([](const auto & info) { return info.name; },                                              \
         [](auto & info, const auto & value) { info.name = value; })
#define gather_enum(name)                                                                          \
  if (items.name)                                                                                  \
  gather([](const auto & info)                                                                     \
         { return static_cast<std::underlying_type_t<decltype(ElemInfo::name)>>(info.name); },     \
         [](auto & info, const auto & value)                                                       \
         { info.name = static_cast<decltype(ElemInfo::name)>(value); })

    // volume
    gather_simple(volume);
    // dim
    gather_simple(dim);
    // elem_mapping_type
    gather_enum(elem_mapping_type);
    // elem_type
    gather_enum(elem_type);
    // hmax
    gather_simple(hmax);
    // hmin
    gather_simple(hmin);
    // neighbor_ids
    gather_simple(neighbor_ids);
    // node_ids
    gather_simple(node_ids);
    // num_sides
    gather_simple(num_sides);
    // points
    gather_simple(points);
    // processor_id
    gather_simple(processor_id);
    // unique_id
    gather_simple(unique_id);

#undef gather_simple
#undef gather_enum
  }
}

template <class CombinedInfosType>
void
MeshInfo::possiblyAddDomainInfo(CombinedInfosType & infos)
{
  constexpr bool is_sidesets = std::is_same_v<CombinedInfosType, SidesetInfos>;
  using id_type = typename CombinedInfosType::id_type;
  using info_type = typename CombinedInfosType::info_type;
  using map_type = typename CombinedInfosType::map_type;

  auto & local = infos.local;
  auto & global = infos.global;

  // Nothing to do
  if (!local && !global)
    return;

  // Helper for either getting an entry from one of the maps or inserting if it doesn't exist
  const auto get_or_insert_info = [](map_type & map, const id_type id) -> info_type &
  { return map.try_emplace(id, id).first->second; };

  // Clear all entries first
  for (auto value_ptr : {local, global})
    if (value_ptr)
      value_ptr->map.clear();

  const auto & items = infos.items;

  // Fill the local information
  map_type local_info;
  {
    const auto compute_volume = items.max_volume || items.min_volume || items.volume;

    // Helper for adding an element to the info
    const auto add = [&](const auto id, const libMesh::Elem & elem, auto && elems_entry)
    {
      auto & entry = get_or_insert_info(local_info, id);
      const Real volume = compute_volume ? elem.volume() : 0;
      if (items.bounding_box)
        entry.bounding_box.union_with(elem.loose_bounding_box());
      if (items.volume)
        entry.volume += volume;
      for (const auto & quality : items.qualities)
        quality.updateValue(entry.qualities, elem.quality(quality.elemQuality()));
      if (items.elems)
        entry.elems.emplace_back(std::move(elems_entry));
      if (items.elem_types)
        entry.elem_types.insert(elem.type());
      if (items.min_volume)
        entry.min_volume = std::min(entry.min_volume, volume);
      if (items.max_volume)
        entry.max_volume = std::max(entry.max_volume, volume);
      if (items.num_elems)
        ++entry.num_elems;
      if (items.processor_ids)
        entry.processor_ids.insert(elem.processor_id());
    };

    // Add elements for subdomains and sidesets
    auto & mesh = _fe_problem.mesh();
    if constexpr (is_sidesets)
    {
      libMesh::ElemSideBuilder side_builder;
      for (const auto & bnd_elem : as_range(mesh.bndElemsBegin(), mesh.bndElemsEnd()))
      {
        const auto & elem = *bnd_elem->_elem;
        if (elem.processor_id() == processor_id())
        {
          const auto side = bnd_elem->_side;
          add(bnd_elem->_bnd_id, side_builder(elem, side), std::make_pair(elem.id(), side));
        }
      }
    }
    else
    {
      for (const auto & elem : *mesh.getActiveLocalElementRange())
        add(elem->subdomain_id(), *elem, elem->id());
    }
  }

  // For local, copy over everything we have. It's just local information.
  if (local)
    local->map = local_info;

  // For global entries, need to accumulate data
  if (global)
  {
    bool did_gather = false;
    const auto gather = [&](const auto && get_value, const auto && set_value)
    {
      using T = std::remove_reference_t<std::invoke_result_t<decltype(get_value), info_type &>>;
      std::vector<std::pair<id_type, T>> data;
      data.reserve(local_info.size());
      for (auto & [id, info] : local_info)
        data.emplace_back(id, get_value(info));
      comm().gather(0, data);
      for (const auto & [id, value] : data)
        set_value(get_or_insert_info(global->map, id), value);
      did_gather = true;
    };

    // bounding_box
    if (items.bounding_box)
      gather([](const auto & info)
             { return std::make_pair(info.bounding_box.min(), info.bounding_box.max()); },
             [](auto & info, const auto & value)
             { info.bounding_box.union_with(BoundingBox(value.first, value.second)); });
    // volume
    if (items.volume)
      gather([](const auto & info) { return info.volume; },
             [](auto & info, const auto & value) { info.volume += value; });
    // qualities
    if (items.qualities.size())
      gather(
          [](const auto & info)
          {
            std::vector<std::tuple<std::underlying_type_t<decltype(DomainQuality::first)>,
                                   std::underlying_type_t<decltype(DomainQuality::second)>,
                                   Real>>
                values;
            values.reserve(info.qualities.size());
            for (const auto & [bounded_quality, value] : info.qualities)
              values.emplace_back(
                  bounded_quality.elemQuality(), bounded_quality.boundType(), value);
            return values;
          },
          [](auto & info, const auto & value)
          {
            for (const auto & [elem_quality, bound_type, quality_value] : value)
            {
              const DomainQuality dq(static_cast<libMesh::ElemQuality>(elem_quality),
                                     static_cast<BoundType>(bound_type));
              dq.updateValue(info.qualities, quality_value);
            }
          });
    // elems
    if (items.elems)
      gather([](const auto & info) { return info.elems; },
             [](auto & info, const auto & value)
             { info.elems.insert(info.elems.end(), value.begin(), value.end()); });
    // elem_types
    if (items.elem_types)
      gather(
          [](const auto & info)
          {
            std::vector<int> values;
            values.reserve(info.elem_types.size());
            std::transform(info.elem_types.begin(),
                           info.elem_types.end(),
                           std::back_inserter(values),
                           [](const auto v) { return static_cast<int>(v); });
            return values;
          },
          [](auto & info, const auto & value)
          {
            std::transform(value.begin(),
                           value.end(),
                           std::inserter(info.elem_types, info.elem_types.end()),
                           [](const auto v) { return static_cast<libMesh::ElemType>(v); });
          });
    // max_volume
    if (items.max_volume)
      gather([](const auto & info) { return info.max_volume; },
             [](auto & info, const auto & value)
             { info.max_volume = std::max(info.max_volume, value); });
    // min_volume
    if (items.min_volume)
      gather([](const auto & info) { return info.min_volume; },
             [](auto & info, const auto & value)
             { info.min_volume = std::min(info.min_volume, value); });
    // num_elems
    if (items.num_elems)
      gather([](const auto & info) { return info.num_elems; },
             [](auto & info, const auto & value) { info.num_elems += value; });
    // processor_ids
    if (items.processor_ids)
      gather(
          [](const auto & info)
          {
            mooseAssert(info.processor_ids.size() == 1, "Should have exactly one pid");
            return *info.processor_ids.begin();
          },
          [](auto & info, const auto & value) { info.processor_ids.insert(value); });

    // If we haven't gathered anything at all (no items), we didn't insert
    // any IDs so we need to do that now
    if (!did_gather)
    {
      std::vector<id_type> data;
      data.reserve(local_info.size());
      std::transform(local_info.begin(),
                     local_info.end(),
                     std::back_inserter(data),
                     [](const auto v) { return v.first; });
      comm().gather(0, data);
      for (const auto id : data)
        get_or_insert_info(global->map, id);
    }

    // For global sidesets, we could technically have sidesets that contain no
    // sides, which we wouldn't have picked up in the local build above. In
    // a previous implementation of MeshInfo, we still reported these. So, keep
    // reporting them.
    if constexpr (is_sidesets)
      for (const auto id : _mesh.get_boundary_info().get_global_boundary_ids())
        get_or_insert_info(global->map, id);
  }

  for (auto to : {local, global})
    if (to)
    {
      // Add sideset/subdomain names
      for (auto & [id, info] : to->map)
        if constexpr (is_sidesets)
          info.name = _mesh.get_boundary_info().get_sideset_name(id);
        else
          info.name = _mesh.subdomain_name(id);

      // Sort "elems" if elems requested
      if (items.elems)
        for (auto & id_info_pair : to->map)
          std::sort(id_info_pair.second.elems.begin(), id_info_pair.second.elems.end());
    }
}

void
MeshInfo::execute()
{
#define set_value(variable, value)                                                                 \
  if (variable)                                                                                    \
  *variable = value

  set_value(_num_dofs, _equation_systems.n_dofs());
  set_value(_num_dofs_nl, _nonlinear_system.n_dofs());
  set_value(_num_dofs_aux, _aux_system.n_dofs());
  if (_num_dofs_constrained)
  {
    *_num_dofs_constrained = 0;
    for (auto s : make_range(_equation_systems.n_systems()))
      *_num_dofs_constrained += _equation_systems.get_system(s).n_constrained_dofs();
  }

  set_value(_num_elem, _mesh.n_elem());
  set_value(_num_node, _mesh.n_nodes());
  set_value(_num_local_dofs, _nonlinear_system.n_local_dofs() + _aux_system.n_local_dofs());
  set_value(_num_local_dofs_nl, _nonlinear_system.n_local_dofs());
  set_value(_num_local_dofs_aux, _aux_system.n_local_dofs());
  set_value(_num_local_elem, _mesh.n_local_elem());
  set_value(_num_local_node, _mesh.n_local_nodes());

#undef set_value

  possiblyAddElemInfo();
  possiblyAddDomainInfo(_sideset_infos);
  possiblyAddDomainInfo(_subdomain_infos);
}

bool
MeshInfo::hasItem(const std::string & name, const MultiMooseEnum & items)
{
  mooseAssert(items.find(name) != items.items().end(), "Invalid item: " << name);
  if (items.isValid())
    return items.isValueSet(name) || items.isValueSet("all");
  return items.find("all") == items.items().end();
}

/// JSON serialization for info maps
#define info_json_simple(name)                                                                     \
  if (items.name)                                                                                  \
  info_json[#name] = info.name
#define info_json_enum(name)                                                                       \
  if (items.name)                                                                                  \
  info_json[#name] = libMesh::Utility::enum_to_string(info.name)
#define info_json_id(name, invalid_value)                                                          \
  if (items.name)                                                                                  \
  {                                                                                                \
    if (info.name == invalid_value)                                                                \
      info_json[#name] = nullptr;                                                                  \
    else                                                                                           \
      info_json[#name] = info.name;                                                                \
  }
#define info_json_ids(name, invalid_value)                                                         \
  if (items.name)                                                                                  \
    for (const auto id : info.name)                                                                \
    {                                                                                              \
      if (id == invalid_value)                                                                     \
        info_json[#name].push_back(nullptr);                                                       \
      else                                                                                         \
        info_json[#name].push_back(id);                                                            \
    }
template <class InfoType>
void
toJSONInfoBase(nlohmann::json & json, const InfoType & info)
{
  json["id"] = info.id;
}
template <class InfoType, class InfoItemsType>
void
toJSONElemContainingInfo(nlohmann::json & info_json,
                         const InfoType & info,
                         const InfoItemsType & items)
{
  toJSONInfoBase(info_json, info);
  info_json_simple(bounding_box);
  info_json_simple(volume);
}
void
to_json(nlohmann::json & json, const MeshInfo::ElemInfoMap & info_map)
{
  const auto & items = info_map.items;
  for (const auto & [id, info] : info_map.map)
  {
    mooseAssert(id == info.id, "Inconsistent id");

    nlohmann::json info_json;
    toJSONElemContainingInfo(info_json, info, items);
    info_json["subdomain_id"] = info.subdomain_id;

    // qualities
    if (info.qualities.size())
      for (const auto & [quality, value] : info.qualities)
        info_json["qualities"][elemQualityToString(quality)] = value;
    // dim
    info_json_simple(dim);
    // elem_mapping_type
    info_json_enum(elem_mapping_type);
    // elem_type
    info_json_enum(elem_type);
    // hmax
    info_json_simple(hmax);
    // hmin
    info_json_simple(hmin);
    // neighbor_ids
    info_json_ids(neighbor_ids, libMesh::DofObject::invalid_id);
    // node_ids
    info_json_ids(node_ids, libMesh::DofObject::invalid_id);
    // num_sides
    info_json_simple(num_sides);
    // points
    info_json_simple(points);
    // processor_id
    info_json_simple(processor_id);
    // unique_id
    info_json_id(unique_id, libMesh::DofObject::invalid_unique_id);

    json.push_back(std::move(info_json));
  }
}
template <class DomainInfoMapType>
void
toJSONDomainInfoMap(nlohmann::json & json, const DomainInfoMapType & info_map)
{
  constexpr bool is_sideset = std::is_same_v<DomainInfoMapType, MeshInfo::SidesetInfoMap>;
  const auto & items = info_map.items;

  for (const auto & [id, info] : info_map.map)
  {
    mooseAssert(id == info.id, "Inconsistent id");

    nlohmann::json info_json;
    toJSONElemContainingInfo(info_json, info, items);

    if (info.name.size())
      info_json["name"] = info.name;
    // qualities
    if (items.qualities.size())
      for (const auto & [bounded_quality, value] : info.qualities)
        info_json["qualities"][bounded_quality.itemName()] = value;
    // elems
    if (items.elems)
    {
      auto & elems_json = info_json["elems"];
      if constexpr (is_sideset)
      {
        for (const auto & elem_entry : info.elems)
          elems_json.push_back({{"elem_id", elem_entry.first}, {"side", elem_entry.second}});
      }
      else
        elems_json = info.elems;
    }
    // elem_types
    if (items.elem_types)
    {
      std::vector<std::string> elem_types;
      elem_types.reserve(info.elem_types.size());
      std::transform(info.elem_types.begin(),
                     info.elem_types.end(),
                     std::back_inserter(elem_types),
                     [](const auto v) { return libMesh::Utility::enum_to_string(v); });
      std::sort(elem_types.begin(), elem_types.end());
      info_json["elem_types"] = elem_types;
    }
    // max_volume
    info_json_simple(max_volume);
    // min_volume
    info_json_simple(min_volume);
    // min_volume
    info_json_simple(num_elems);
    // processor_ids
    info_json_simple(processor_ids);

    json.push_back(std::move(info_json));
  }
}
#undef info_json_simple
#undef info_json_enum
#undef info_json_id
#undef info_json_ids
void
to_json(nlohmann::json & json, const MeshInfo::SidesetInfoMap & info_map)
{
  toJSONDomainInfoMap(json, info_map);
}
void
to_json(nlohmann::json & json, const MeshInfo::SubdomainInfoMap & info_map)
{
  toJSONDomainInfoMap(json, info_map);
}

/// Data store and load for DomainQuality
void
dataStore(std::ostream & stream, MeshInfo::DomainQuality & beq, void *)
{
  int value;

  value = static_cast<int>(beq.elemQuality());
  dataStore(stream, value, nullptr);

  value = static_cast<int>(beq.boundType());
  dataStore(stream, value, nullptr);
}
void
dataLoad(std::istream & stream, MeshInfo::DomainQuality & beq, void *)
{
  int value;

  dataLoad(stream, value, nullptr);
  beq.elemQuality() = static_cast<libMesh::ElemQuality>(value);

  dataLoad(stream, value, nullptr);
  beq.boundType() = static_cast<MeshInfo::BoundType>(value);
}

/// Data store and load for node, elem, sideset, subdomain info entries
template <typename T>
void
dataStoreInfoBase(std::ostream & stream, T & info)
{
  dataStore(stream, info.id, nullptr);
}
template <typename T>
void
dataStoreElemContainingInfo(std::ostream & stream, T & info)
{
  dataStoreInfoBase(stream, info);
  dataStore(stream, info.bounding_box, nullptr);
  dataStore(stream, info.volume, nullptr);
}
void
dataStore(std::ostream & stream, MeshInfo::ElemInfo & info, void *)
{
  dataStoreElemContainingInfo(stream, info);
  dataStore(stream, info.qualities, nullptr);
  dataStore(stream, info.dim, nullptr);
  dataStore(stream, info.elem_mapping_type, nullptr);
  dataStore(stream, info.elem_type, nullptr);
  dataStore(stream, info.hmax, nullptr);
  dataStore(stream, info.hmin, nullptr);
  dataStore(stream, info.neighbor_ids, nullptr);
  dataStore(stream, info.node_ids, nullptr);
  dataStore(stream, info.num_sides, nullptr);
  dataStore(stream, info.points, nullptr);
  dataStore(stream, info.processor_id, nullptr);
  dataStore(stream, info.subdomain_id, nullptr);
  dataStore(stream, info.unique_id, nullptr);
}
template <typename T>
void
dataStoreDomainInfo(std::ostream & stream, T & info)
{
  dataStoreElemContainingInfo(stream, info);
  dataStore(stream, info.name, nullptr);
  dataStore(stream, info.qualities, nullptr);
  dataStore(stream, info.elems, nullptr);
  dataStore(stream, info.elem_types, nullptr);
  dataStore(stream, info.min_volume, nullptr);
  dataStore(stream, info.max_volume, nullptr);
  dataStore(stream, info.num_elems, nullptr);
  dataStore(stream, info.processor_ids, nullptr);
}
void
dataStore(std::ostream & stream, MeshInfo::SidesetInfo & info, void *)
{
  dataStoreDomainInfo(stream, info);
}
void
dataStore(std::ostream & stream, MeshInfo::SubdomainInfo & info, void *)
{
  dataStoreDomainInfo(stream, info);
}
template <typename T>
void
dataLoadInfoBase(std::istream & stream, T & info)
{
  dataLoad(stream, info.id, nullptr);
}
template <typename T>
void
dataLoadElemContainingInfo(std::istream & stream, T & info)
{
  dataLoadInfoBase(stream, info);
  dataLoad(stream, info.bounding_box, nullptr);
  dataLoad(stream, info.volume, nullptr);
}
void
dataLoad(std::istream & stream, MeshInfo::ElemInfo & info, void *)
{
  dataLoadElemContainingInfo(stream, info);
  dataLoad(stream, info.qualities, nullptr);
  dataLoad(stream, info.dim, nullptr);
  dataLoad(stream, info.elem_mapping_type, nullptr);
  dataLoad(stream, info.elem_type, nullptr);
  dataLoad(stream, info.hmax, nullptr);
  dataLoad(stream, info.hmin, nullptr);
  dataLoad(stream, info.neighbor_ids, nullptr);
  dataLoad(stream, info.node_ids, nullptr);
  dataLoad(stream, info.num_sides, nullptr);
  dataLoad(stream, info.points, nullptr);
  dataLoad(stream, info.processor_id, nullptr);
  dataLoad(stream, info.subdomain_id, nullptr);
  dataLoad(stream, info.unique_id, nullptr);
}
template <typename T>
void
dataLoadDomainInfo(std::istream & stream, T & info)
{
  dataLoadElemContainingInfo(stream, info);
  dataLoad(stream, info.name, nullptr);
  dataLoad(stream, info.qualities, nullptr);
  dataLoad(stream, info.elems, nullptr);
  dataLoad(stream, info.elem_types, nullptr);
  dataLoad(stream, info.min_volume, nullptr);
  dataLoad(stream, info.max_volume, nullptr);
  dataLoad(stream, info.num_elems, nullptr);
  dataLoad(stream, info.processor_ids, nullptr);
}
void
dataLoad(std::istream & stream, MeshInfo::SidesetInfo & info, void *)
{
  dataLoadDomainInfo(stream, info);
}
void
dataLoad(std::istream & stream, MeshInfo::SubdomainInfo & info, void *)
{
  dataLoadDomainInfo(stream, info);
}

/// Data store and load for info maps
void
dataStore(std::ostream & stream, MeshInfo::ElemInfoMap & info_map, void *)
{
  dataStore(stream, info_map.map, nullptr);
}
void
dataStore(std::ostream & stream, MeshInfo::SidesetInfoMap & info_map, void *)
{
  dataStore(stream, info_map.map, nullptr);
}
void
dataStore(std::ostream & stream, MeshInfo::SubdomainInfoMap & info_map, void *)
{
  dataStore(stream, info_map.map, nullptr);
}
void
dataLoad(std::istream & stream, MeshInfo::ElemInfoMap & info_map, void *)
{
  dataLoad(stream, info_map.map, nullptr);
}
void
dataLoad(std::istream & stream, MeshInfo::SidesetInfoMap & info_map, void *)
{
  dataLoad(stream, info_map.map, nullptr);
}
void
dataLoad(std::istream & stream, MeshInfo::SubdomainInfoMap & info_map, void *)
{
  dataLoad(stream, info_map.map, nullptr);
}

template MeshInfo::SidesetInfos MeshInfo::initCombinedInfos<MeshInfo::SidesetInfos>(
    const std::string &, const MultiMooseEnum &, const MultiMooseEnum &);
template MeshInfo::SubdomainInfos MeshInfo::initCombinedInfos<MeshInfo::SubdomainInfos>(
    const std::string &, const MultiMooseEnum &, const MultiMooseEnum &);
template void MeshInfo::possiblyAddDomainInfo<MeshInfo::SidesetInfos>(MeshInfo::SidesetInfos &);
template void MeshInfo::possiblyAddDomainInfo<MeshInfo::SubdomainInfos>(MeshInfo::SubdomainInfos &);
