#include "PatternedHexMeshGenerator.h"
#include "HexIDPatternedMeshGenerator.h"

registerMooseObject("ReactorApp", HexIDPatternedMeshGenerator);

InputParameters
HexIDPatternedMeshGenerator::validParams()
{
  InputParameters params = PatternedHexMeshGenerator::validParams();
  params.addRequiredParam<std::string>("id_name", "Name of extra integer ID set");
  params.addParam<std::vector<MeshGeneratorName>>(
      "exclude_id", "Name of input meshes to be excluded in ID generation");
  MooseEnum option("cell pattern manual", "cell");
  params.addParam<MooseEnum>("assign_type", option, "Type of integer ID assignment");
  params.addParam<std::vector<std::vector<dof_id_type>>>(
      "id_pattern",
      "User-defined element IDs. A double-indexed array starting with the upper-left corner");
  params.addClassDescription("Generate hexagonal lattice meshes with reporting (extra integer) ID "
                             "assignments that indentifies individual components of lattice.");
  return params;
}

HexIDPatternedMeshGenerator::HexIDPatternedMeshGenerator(const InputParameters & parameters)
  : PatternedHexMeshGenerator(parameters),
    _element_id_name(getParam<std::string>("id_name")),
    _assign_type(getParam<MooseEnum>("assign_type")),
    _use_exclude_id(isParamValid("exclude_id"))
{
  if (_use_exclude_id && _assign_type != "cell")
    paramError("exclude_id", "works only when \"assign_type\" is equal 'cell'");
  if (!isParamValid("id_pattern") && _assign_type == "manual")
    paramError("id_pattern", "required when \"assign_type\" is equal to 'manual'");

  if (_assign_type == "manual")
    _id_pattern = getParam<std::vector<std::vector<dof_id_type>>>("id_pattern");
  _exclude_ids.resize(_input_names.size());
  if (_use_exclude_id)
  {
    std::vector<MeshGeneratorName> exclude_id_name =
        getParam<std::vector<MeshGeneratorName>>("exclude_id");
    for (unsigned int i = 0; i < _input_names.size(); ++i)
    {
      _exclude_ids[i] = false;
      for (auto input_name : exclude_id_name)
        if (_input_names[i] == input_name)
        {
          _exclude_ids[i] = true;
          break;
        }
    }
  }
  else
  {
    for (unsigned int i = 0; i < _input_names.size(); ++i)
      _exclude_ids[i] = false;
  }
}

std::unique_ptr<MeshBase>
HexIDPatternedMeshGenerator::generate()
{
  auto mesh = PatternedHexMeshGenerator::generate();
  std::vector<dof_id_type> integer_ids;
  if (_assign_type == "cell")
    integer_ids = getCellwiseIntegerIDs();
  else if (_assign_type == "pattern")
    integer_ids = getPatternIntegerIDs();
  else if (_assign_type == "manual")
    integer_ids = getManualIntegerIDs();

  std::set<SubdomainID> blks = getCellBlockIDs();
  unsigned int duct_boundary_id = *std::max_element(integer_ids.begin(), integer_ids.end()) + 1;

  std::map<SubdomainID, unsigned int> blks_duct = getDuckBlockIDs(mesh, blks);

  unsigned int extra_id_index = mesh->add_elem_integer(_element_id_name);

  unsigned int i = 0;
  unsigned int id = integer_ids[i];
  unsigned old_id = id;
  for (auto elem : mesh->element_ptr_range())
  {
    auto blk = elem->subdomain_id();
    auto it = blks.find(blk);
    if (it == blks.end())
    {
      if (_has_assembly_duct)
      {
        auto it2 = blks_duct.find(blk);
        if (it2 == blks_duct.end())
          elem->set_extra_integer(extra_id_index, old_id);
        else
          elem->set_extra_integer(extra_id_index, duct_boundary_id + it2->second);
      }
      else
        elem->set_extra_integer(extra_id_index, old_id);
    }
    else
    {
      elem->set_extra_integer(extra_id_index, id);
      ++i;
      old_id = id;
      id = integer_ids[i];
    }
  }
  return dynamic_pointer_cast<MeshBase>(mesh);
}

std::vector<dof_id_type>
HexIDPatternedMeshGenerator::getCellwiseIntegerIDs() const
{
  std::vector<dof_id_type> integer_ids;
  dof_id_type id = 0;
  for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
  {
    for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
    {
      // ReplicatedMesh & cell_mesh = *_meshes[_pattern[i][j]];
      const ReplicatedMesh * cell_mesh =
          dynamic_cast<ReplicatedMesh *>((*_mesh_ptrs[_pattern[i][j]]).get());
      unsigned int n_cell_elem = cell_mesh->n_elem();
      bool exclude_id = false;
      if (_use_exclude_id)
        if (_exclude_ids[_pattern[i][j]])
          exclude_id = true;
      if (!exclude_id)
      {
        for (unsigned int k = 0; k < n_cell_elem; ++k)
          integer_ids.push_back(id);
        ++id;
      }
      else
        for (unsigned int k = 0; k < n_cell_elem; ++k)
          integer_ids.push_back(DofObject::invalid_id);
    }
  }
  return integer_ids;
}

std::vector<dof_id_type>
HexIDPatternedMeshGenerator::getPatternIntegerIDs() const
{
  std::vector<dof_id_type> integer_ids;
  for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
  {
    for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
    {
      const ReplicatedMesh * cell_mesh =
          dynamic_cast<ReplicatedMesh *>((*_mesh_ptrs[_pattern[i][j]]).get());
      unsigned int n_cell_elem = cell_mesh->n_elem();
      for (unsigned int k = 0; k < n_cell_elem; ++k)
        integer_ids.push_back(_pattern[i][j]);
    }
  }
  return integer_ids;
}
std::vector<dof_id_type>
HexIDPatternedMeshGenerator::getManualIntegerIDs() const
{
  std::vector<dof_id_type> integer_ids;
  for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
  {
    for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
    {
      unsigned int id = _id_pattern[i][j];
      const ReplicatedMesh * cell_mesh =
          dynamic_cast<ReplicatedMesh *>((*_mesh_ptrs[_pattern[i][j]]).get());
      unsigned int n_cell_elem = cell_mesh->n_elem();
      for (unsigned int k = 0; k < n_cell_elem; ++k)
        integer_ids.push_back(id);
    }
  }
  return integer_ids;
}

std::set<SubdomainID>
HexIDPatternedMeshGenerator::getCellBlockIDs() const
{
  std::set<SubdomainID> blks;
  for (MooseIndex(_pattern) i = 0; i < _pattern.size(); ++i)
  {
    for (MooseIndex(_pattern[i]) j = 0; j < _pattern[i].size(); ++j)
    {
      const ReplicatedMesh * cell_mesh =
          dynamic_cast<ReplicatedMesh *>((*_mesh_ptrs[_pattern[i][j]]).get());
      for (auto elem : cell_mesh->element_ptr_range())
      {
        auto blk = elem->subdomain_id();
        auto it = blks.find(blk);
        if (it == blks.end())
          blks.insert(blk);
      }
    }
  }
  return blks;
}

std::map<SubdomainID, unsigned int>
HexIDPatternedMeshGenerator::getDuckBlockIDs(std::unique_ptr<MeshBase> & mesh,
                                             std::set<SubdomainID> & blks) const
{
  unsigned int i = 0;
  std::map<SubdomainID, unsigned int> blks_duct;
  if (_has_assembly_duct)
  {
    for (auto elem : mesh->element_ptr_range())
    {
      auto blk = elem->subdomain_id();
      auto it1 = blks.find(blk);
      if (it1 == blks.end())
      {
        auto it2 = blks_duct.find(blk);
        if (it2 == blks_duct.end())
          blks_duct[blk] = i++;
      }
    }
    blks_duct.erase(blks_duct.begin());
  }
  return blks_duct;
}
