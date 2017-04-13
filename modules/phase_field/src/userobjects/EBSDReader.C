/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "EBSDReader.h"
#include "EBSDMesh.h"
#include "MooseMesh.h"
#include "Conversion.h"
#include "NonlinearSystem.h"

template <>
InputParameters
validParams<EBSDReader>()
{
  InputParameters params = validParams<EulerAngleProvider>();
  params.addClassDescription("Load and manage DREAM.3D EBSD data files for running simulations on "
                             "reconstructed microstructures.");
  params.addParam<unsigned int>(
      "custom_columns", 0, "Number of additional custom data columns to read from the EBSD file");
  return params;
}

EBSDReader::EBSDReader(const InputParameters & params)
  : EulerAngleProvider(params),
    _mesh(_fe_problem.mesh()),
    _nl(_fe_problem.getNonlinearSystemBase()),
    _grain_num(0),
    _custom_columns(getParam<unsigned int>("custom_columns")),
    _time_step(_fe_problem.timeStep()),
    _mesh_dimension(_mesh.dimension()),
    _nx(0),
    _ny(0),
    _nz(0),
    _dx(0.),
    _dy(0.),
    _dz(0.)
{
  readFile();
}

void
EBSDReader::readFile()
{
  // No need to re-read data upon recovery
  if (_app.isRecovering())
    return;

  // Fetch and check mesh
  EBSDMesh * mesh = dynamic_cast<EBSDMesh *>(&_mesh);
  if (mesh == NULL)
    mooseError("Please use an EBSDMesh in your simulation.");

  std::ifstream stream_in(mesh->getEBSDFilename().c_str());
  if (!stream_in)
    mooseError("Can't open EBSD file: ", mesh->getEBSDFilename());

  const EBSDMesh::EBSDMeshGeometry & g = mesh->getEBSDGeometry();

  // Copy file header data from the EBSDMesh
  _dx = g.d[0];
  _nx = g.n[0];
  _minx = g.min[0];
  _maxx = _minx + _dx * _nx;

  _dy = g.d[1];
  _ny = g.n[1];
  _miny = g.min[1];
  _maxy = _miny + _dy * _ny;

  _dz = g.d[2];
  _nz = g.n[2];
  _minz = g.min[2];
  _maxz = _minz + _dz * _nz;

  // Resize the _data array
  unsigned total_size = g.dim < 3 ? _nx * _ny : _nx * _ny * _nz;
  _data.resize(total_size);

  std::string line;
  while (std::getline(stream_in, line))
  {
    if (line.find("#") != 0)
    {
      // Temporary variables to read in on each line
      EBSDPointData d;
      Real x, y, z;

      std::istringstream iss(line);
      iss >> d._phi1 >> d._Phi >> d._phi2 >> x >> y >> z >> d._feature_id >> d._phase >>
          d._symmetry;

      // Transform angles to degrees
      d._phi1 *= 180.0 / libMesh::pi;
      d._Phi *= 180.0 / libMesh::pi;
      d._phi2 *= 180.0 / libMesh::pi;

      // Custom columns
      d._custom.resize(_custom_columns);
      for (unsigned int i = 0; i < _custom_columns; ++i)
        if (!(iss >> d._custom[i]))
          mooseError("Unable to read in EBSD custom data column #", i);

      if (x < _minx || y < _miny || x > _maxx || y > _maxy ||
          (g.dim == 3 && (z < _minz || z > _maxz)))
        mooseError("EBSD Data ouside of the domain declared in the header ([",
                   _minx,
                   ':',
                   _maxx,
                   "], [",
                   _miny,
                   ':',
                   _maxy,
                   "], [",
                   _minz,
                   ':',
                   _maxz,
                   "]) dim=",
                   g.dim,
                   "\n",
                   line);

      d._p = Point(x, y, z);

      // determine number of grains in the dataset
      if (_global_id_map.find(d._feature_id) == _global_id_map.end())
        _global_id_map[d._feature_id] = _grain_num++;

      unsigned int global_index = indexFromPoint(Point(x, y, z));
      _data[global_index] = d;
    }
  }
  stream_in.close();

  // Resize the variables
  _avg_data.resize(_grain_num);
  _avg_angles.resize(_grain_num);

  // clear the averages
  for (unsigned int i = 0; i < _grain_num; ++i)
  {
    EBSDAvgData & a = _avg_data[i];
    a._symmetry = a._phase = a._n = 0;
    a._p = 0.0;
    a._custom.assign(_custom_columns, 0.0);

    EulerAngles & b = _avg_angles[i];
    b.phi1 = b.Phi = b.phi2 = 0.0;
  }

  // Iterate through data points to get average variable values for each grain
  for (auto & j : _data)
  {
    EBSDAvgData & a = _avg_data[_global_id_map[j._feature_id]];
    EulerAngles & b = _avg_angles[_global_id_map[j._feature_id]];

    // use Eigen::Quaternion<Real> here?
    b.phi1 += j._phi1;
    b.Phi += j._Phi;
    b.phi2 += j._phi2;

    if (a._n == 0)
      a._phase = j._phase;
    else if (a._phase != j._phase)
      mooseError("An EBSD feature needs to have a uniform phase.");

    if (a._n == 0)
      a._symmetry = j._symmetry;
    else if (a._symmetry != j._symmetry)
      mooseError("An EBSD feature needs to have a uniform symmetry parameter.");

    for (unsigned int i = 0; i < _custom_columns; ++i)
      a._custom[i] += j._custom[i];

    // store the feature (or grain) ID
    a._feature_id = j._feature_id;

    a._p += j._p;
    a._n++;
  }

  for (unsigned int i = 0; i < _grain_num; ++i)
  {
    EBSDAvgData & a = _avg_data[i];
    EulerAngles & b = _avg_angles[i];

    if (a._n == 0)
      continue;

    // TODO: need better way to average angles
    b.phi1 /= Real(a._n);
    b.Phi /= Real(a._n);
    b.phi2 /= Real(a._n);

    // link the EulerAngles into the EBSDAvgData for access via the functors
    a._angles = &b;

    if (a._phase >= _global_id.size())
      _global_id.resize(a._phase + 1);

    // The averaged per grain data locally contains the phase id, local id, and
    // original feature id. It is stored contiguously indexed by global id.
    a._local_id = _global_id[a._phase].size();
    _global_id[a._phase].push_back(i);

    a._p *= 1.0 / Real(a._n);

    for (unsigned int i = 0; i < _custom_columns; ++i)
      a._custom[i] /= Real(a._n);
  }

  // Build maps to indicate the weights with which grain and phase data
  // from the surrounding elements contributes to a node fo IC purposes
  buildNodeWeightMaps();
}

EBSDReader::~EBSDReader() {}

const EBSDReader::EBSDPointData &
EBSDReader::getData(const Point & p) const
{
  return _data[indexFromPoint(p)];
}

const EBSDReader::EBSDAvgData &
EBSDReader::getAvgData(unsigned int var) const
{
  return _avg_data[indexFromIndex(var)];
}

const EulerAngles &
EBSDReader::getEulerAngles(unsigned int var) const
{
  return _avg_angles[indexFromIndex(var)];
}

const EBSDReader::EBSDAvgData &
EBSDReader::getAvgData(unsigned int phase, unsigned int local_id) const
{
  return _avg_data[indexFromIndex(_global_id[phase][local_id])];
}

unsigned int
EBSDReader::getGrainNum() const
{
  return _grain_num;
}

unsigned int
EBSDReader::getGrainNum(unsigned int phase) const
{
  return _global_id[phase].size();
}

unsigned int
EBSDReader::indexFromPoint(const Point & p) const
{
  // Don't assume an ordering on the input data, use the (x, y,
  // z) values of this centroid to determine the index.
  unsigned int x_index, y_index, z_index, global_index;

  x_index = (unsigned int)((p(0) - _minx) / _dx);
  y_index = (unsigned int)((p(1) - _miny) / _dy);

  if (_mesh_dimension == 3)
  {
    z_index = (unsigned int)((p(2) - _minz) / _dz);
    global_index = z_index * _ny;
  }
  else
    global_index = 0;

  // Compute the global index into the _data array.  This stores points
  // in a [z][y][x] ordering.
  global_index = (global_index + y_index) * _nx + x_index;

  // Don't access out of range!
  mooseAssert(global_index < _data.size(), "global_index points out of _data range");

  return global_index;
}

unsigned int
EBSDReader::indexFromIndex(unsigned int var) const
{

  // Transfer the index into the _avg_data array.
  unsigned avg_index = var;

  // Don't access out of range!
  if (avg_index >= _avg_data.size())
    mooseError("Error! Index out of range in EBSDReader::indexFromIndex(), index: ",
               avg_index,
               " size: ",
               _avg_data.size());

  return avg_index;
}

const std::map<dof_id_type, std::vector<Real>> &
EBSDReader::getNodeToGrainWeightMap() const
{
  return _node_to_grain_weight_map;
}

const std::map<dof_id_type, std::vector<Real>> &
EBSDReader::getNodeToPhaseWeightMap() const
{
  return _node_to_phase_weight_map;
}

unsigned int
EBSDReader::getGlobalID(unsigned int feature_id) const
{
  auto it = _global_id_map.find(feature_id);
  if (it == _global_id_map.end())
    mooseError("Invalid Feature ID");
  return it->second;
}

void
EBSDReader::meshChanged()
{
  // maps are only rebuild for use in initial conditions, which happens in time step zero
  if (_time_step == 0)
    buildNodeWeightMaps();
}

void
EBSDReader::buildNodeWeightMaps()
{
  // Import nodeToElemMap from MooseMesh for current node
  // This map consists of the node index followed by a vector of element indices that are associated
  // with that node
  const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map =
      _mesh.nodeToActiveSemilocalElemMap();
  libMesh::MeshBase & mesh = _mesh.getMesh();

  // Loop through each node in mesh and calculate eta values for each grain associated with the node
  MeshBase::const_node_iterator ni = mesh.active_nodes_begin();
  const MeshBase::const_node_iterator nend = mesh.active_nodes_end();
  for (; ni != nend; ++ni)
  {
    // Get node_id
    const dof_id_type node_id = (*ni)->id();

    // Initialize map entries for current node
    _node_to_grain_weight_map[node_id].assign(getGrainNum(), 0.0);
    _node_to_phase_weight_map[node_id].assign(getPhaseNum(), 0.0);

    // Loop through element indices associated with the current node and record weighted eta value
    // in new map
    const auto & node_to_elem_pair = node_to_elem_map.find(node_id);
    if (node_to_elem_pair != node_to_elem_map.end())
    {
      unsigned int n_elems =
          node_to_elem_pair->second
              .size(); // n_elems can range from 1 to 4 for 2D and 1 to 8 for 3D problems

      for (unsigned int ne = 0; ne < n_elems; ++ne)
      {
        // Current element index
        unsigned int elem_id = (node_to_elem_pair->second)[ne];

        // Retrieve EBSD grain number for the current element index
        const Elem * elem = mesh.elem(elem_id);
        const EBSDReader::EBSDPointData & d = getData(elem->centroid());

        // get the (global) grain ID for the EBSD feature ID
        const unsigned int global_id = getGlobalID(d._feature_id);

        // Calculate eta value and add to map
        _node_to_grain_weight_map[node_id][global_id] += 1.0 / n_elems;
        _node_to_phase_weight_map[node_id][d._phase] += 1.0 / n_elems;
      }
    }
  }
}

MooseSharedPointer<EBSDAccessFunctors::EBSDPointDataFunctor>
EBSDReader::getPointDataAccessFunctor(const MooseEnum & field_name) const
{
  EBSDPointDataFunctor * ret_val = NULL;

  switch (field_name)
  {
    case 0: // phi1
      ret_val = new EBSDPointDataPhi1();
      break;
    case 1: // phi
      ret_val = new EBSDPointDataPhi();
      break;
    case 2: // phi2
      ret_val = new EBSDPointDataPhi2();
      break;
    case 3: // grain
      ret_val = new EBSDPointDataFeatureID();
      break;
    case 4: // phase
      ret_val = new EBSDPointDataPhase();
      break;
    case 5: // symmetry
      ret_val = new EBSDPointDataSymmetry();
      break;
    default:
    {
      // check for custom columns
      for (unsigned int i = 0; i < _custom_columns; ++i)
        if (field_name == "CUSTOM" + Moose::stringify(i))
        {
          ret_val = new EBSDPointDataCustom(i);
          break;
        }
    }
  }

  // If ret_val was not set by any of the above cases, throw an error.
  if (!ret_val)
    mooseError("Error:  Please input supported EBSD_param");

  // If we made it here, wrap up the the ret_val in a
  // MooseSharedPointer and ship it off.
  return MooseSharedPointer<EBSDPointDataFunctor>(ret_val);
}

MooseSharedPointer<EBSDAccessFunctors::EBSDAvgDataFunctor>
EBSDReader::getAvgDataAccessFunctor(const MooseEnum & field_name) const
{
  EBSDAvgDataFunctor * ret_val = NULL;

  switch (field_name)
  {
    case 0: // phi1
      ret_val = new EBSDAvgDataPhi1();
      break;
    case 1: // phi
      ret_val = new EBSDAvgDataPhi();
      break;
    case 2: // phi2
      ret_val = new EBSDAvgDataPhi2();
      break;
    case 3: // phase
      ret_val = new EBSDAvgDataPhase();
      break;
    case 4: // symmetry
      ret_val = new EBSDAvgDataSymmetry();
      break;
    case 5: // local_id
      ret_val = new EBSDAvgDataLocalID();
      break;
    case 6: // feature_id
      ret_val = new EBSDAvgDataFeatureID();
      break;
    default:
    {
      // check for custom columns
      for (unsigned int i = 0; i < _custom_columns; ++i)
        if (field_name == "CUSTOM" + Moose::stringify(i))
        {
          ret_val = new EBSDAvgDataCustom(i);
          break;
        }
    }
  }

  // If ret_val was not set by any of the above cases, throw an error.
  if (!ret_val)
    mooseError("Error:  Please input supported EBSD_param");

  // If we made it here, wrap up the the ret_val in a
  // MooseSharedPointer and ship it off.
  return MooseSharedPointer<EBSDAvgDataFunctor>(ret_val);
}
