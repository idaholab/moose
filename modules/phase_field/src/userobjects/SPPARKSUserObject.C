#include "SPPARKSUserObject.h"

#include "libmesh/mesh_tools.h"

#include <numeric>

template<>
InputParameters validParams<SPPARKSUserObject>()
{
  InputParameters params = validParams<GeneralUserObject>();
  params.addParam<std::string>("file", "", "SPPARKS input file");
  params.addParam<bool>("spparks_only", false, "Whether to run SPPARKS independently of ELK");
  params.addParam<std::vector<unsigned> >("ivar", "Index into SPPARKS iarray.  This data will be extracted from SPPARKS.");
  params.addParam<std::vector<unsigned> >("dvar", "Index into SPPARKS darray.  This data will be extracted from SPPARKS.");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("zmin", 0.0, "Lower Z Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  params.addParam<Real>("zmax", 1.0, "Upper Z Coordinate of the generated mesh");

  // Hide from input file dump
  // params.addPrivateParam<std::string>("built_by_action", "" );
  return params;
}

SPPARKSUserObject::SPPARKSUserObject(const std::string & name, InputParameters params)
  :GeneralUserObject(name, params),
   _spparks(NULL),
   _file(getParam<std::string>("file")),
   _spparks_only(getParam<bool>("spparks_only")),
   _ivar(isParamValid("ivar") ? getParam<std::vector<unsigned> >("ivar") : std::vector<unsigned>()),
   _dvar(isParamValid("dvar") ? getParam<std::vector<unsigned> >("dvar") : std::vector<unsigned>()),
   _xmin(getParam<Real>("xmin")),
   _ymin(getParam<Real>("ymin")),
   _zmin(getParam<Real>("zmin")),
   _xmax(getParam<Real>("xmax")),
   _ymax(getParam<Real>("ymax")),
   _zmax(getParam<Real>("zmax")),
   _last_time(std::numeric_limits<Real>::min())
{

  std::cout << std::endl
            << ">>>> STARTING SPPARKS <<<<" << std::endl;
  spparks_open( 0, NULL, libMesh::COMM_WORLD, &_spparks );
  if (!_spparks)
  {
    mooseError("Error initializing SPPARKS");
  }

  char * file = new char[_file.length()+1];
  std::strcpy( file, _file.c_str() );
  std::cout << std::endl
            << ">>>> RUNNING SPPARKS FILE " << _file << " <<<<" << std::endl;
  spparks_file(_spparks, file);

  // Extract and print information about the SPPARKS internals
  int * iptr;
  char dimension[] = "dimension";
  getSPPARKSPointer(iptr, dimension);
  _dim = *iptr;
  std::cerr << std::endl
            << ">>>> SPPARKS DIMENSION: " << _dim << " <<<<" << std::endl;

  double * dptr;
  char boxxlo[] = "boxxlo";
  getSPPARKSPointer(dptr, boxxlo);
  double xlo = *dptr;
  std::cerr << std::endl
            << ">>>> SPPARKS BOXXLO: " << xlo << " <<<<" << std::endl;

  char nlocal[] = "nlocal";
  getSPPARKSPointer(iptr, nlocal);
  int nlcl = *iptr;
  std::cerr << std::endl
            << ">>>> SPPARKS NLOCAL: " << nlcl << " <<<<" << std::endl;

  char id[] = "id";
  getSPPARKSPointer(iptr, id);
  int * id_array = iptr;
  std::cerr << std::endl
            << ">>>> SPPARKS ID: " << id_array << " <<<<" << std::endl;

  double ** ddptr;
  char xyz[] = "xyz";
  getSPPARKSPointer(ddptr, xyz);
  double ** xyz_array = ddptr;
  std::cerr << std::endl
            << ">>>> SPPARKS XYZ: " << xyz_array << " <<<<" << std::endl;
  // for (unsigned i = 0; i < nlcl; ++i)
  // {
  //   std::cout << id_array[i] << "\t"
  //             << xyz_array[i][0] << " "
  //             << xyz_array[i][1] << " "
  //             << xyz_array[i][2] << std::endl;
  // }

  delete[] file;
}

SPPARKSUserObject::~SPPARKSUserObject()
{
  spparks_close( _spparks );
}

int
SPPARKSUserObject::getIntValue( unsigned elk_node_id, unsigned index ) const
{
  if (_spparks_only)
  {
    return 0;
  }

  return getValue( _int_data_for_elk, elk_node_id, index );
}

Real
SPPARKSUserObject::getDoubleValue( unsigned elk_node_id, unsigned index ) const
{
  if (_spparks_only)
  {
    return 0;
  }

  return getValue( _double_data_for_elk, elk_node_id, index );
}

void
SPPARKSUserObject::initialize()
{
  if (_spparks_only)
  {
    return;
  }

  getSPPARKSData();
}

void
SPPARKSUserObject::getSPPARKSData()
{
  // Update the integer data
  char iarray[] = "iarray";
  for (unsigned i = 0; i < _ivar.size(); ++i)
  {
    getSPPARKSData( _int_data_for_elk[_ivar[i]], iarray, _ivar[i] );
  }

  // Update the double data
  char darray[] = "darray";
  for (unsigned i = 0; i < _dvar.size(); ++i)
  {
    getSPPARKSData( _double_data_for_elk[_dvar[i]], darray, _dvar[i] );
  }
}

void
SPPARKSUserObject::execute()
{
  if (_spparks_only)
  {
    return;
  }

  if (_t != _last_time)
  {
    _last_time = _t;

    getSPPARKSData();
  }
}

void
SPPARKSUserObject::initialSetup()
{
  if (_spparks_only)
  {
    return;
  }


  // Initialize communication maps

  // 1. Get on-processor map from SPPARKS ID to ELK ID
  int * iptr;
  char nlocal[] = "nlocal";
  getSPPARKSPointer(iptr, nlocal);
  int nlcl = *iptr;

  char id[] = "id";
  getSPPARKSPointer(iptr, id);
  int * id_array = iptr;

  double ** ddptr;
  char xyz[] = "xyz";
  getSPPARKSPointer(ddptr, xyz);
  double ** xyz_array = ddptr;

  std::set<SPPARKSID> spparks_id;
  for (int i = 0; i < nlcl; ++i)
  {
    Point p(xyz_array[i][0],
            _dim > 1 ? xyz_array[i][1] : 0,
            _dim > 2 ? xyz_array[i][2] : 0);
    spparks_id.insert( SPPARKSID( i, p ) );
  }

  std::multiset<ELKID> elk_id;
  ConstNodeRange & node_range = *_fe_problem.mesh().getLocalNodeRange();
  for ( ConstNodeRange::const_iterator i = node_range.begin(); i < node_range.end(); ++i )
  {
    Point coor( **i );
    if (coor(0) == _xmax)
    {
      coor(0) = _xmin;
    }
    if (coor(1) == _ymax)
    {
      coor(1) = _ymin;
    }
    if (coor(2) == _zmax)
    {
      coor(2) = _zmin;
    }
    elk_id.insert( ELKID( (*i)->id(), coor ) );
  }
  _num_local_elk_nodes = elk_id.size();

  std::set<SPPARKSID> unmatched_spparks;
  for (std::set<SPPARKSID>::iterator i = spparks_id.begin(); i != spparks_id.end(); ++i)
  {
    std::multiset<ELKID>::iterator elk_iter = elk_id.find( *i );
    if (elk_iter != elk_id.end() )
    {
      _spparks_to_elk.insert(std::make_pair<SPPARKSID, ELKID>(*i, *elk_iter));
    }
    else
    {
      unmatched_spparks.insert(*i);
    }
  }
  for (std::multiset<ELKID>::iterator i = elk_id.begin(); i != elk_id.end(); ++i)
  {
    std::set<SPPARKSID>::iterator spparks_iter = spparks_id.find( *i );
    if (spparks_iter != spparks_id.end() )
    {
      _elk_to_spparks.insert(std::make_pair<ELKID, SPPARKSID>(*i, *spparks_iter));
    }
    // else
    // {
    //   _spparks_to_proc.insert(std::make_pair<SPPARKSID, unsigned>(*i, -1));
    // }
  }

  const unsigned num_procs = libMesh::n_processors();

  if (num_procs == 1)
  {
    if (spparks_id.size() != _spparks_to_elk.size())
    {
      mooseError("Did not find ELK node for each SPPARKS node");
    }
    return;
  }




  // 2. Get send map (spparks id -> proc id)

  // A. Get local SPPARKS bounding box
  // TODO: Expand bounding boxes by half cell width?
  //       May not be necessary.

  unsigned proc_id = libMesh::processor_id();

  std::vector<Real> spparks_bounds(num_procs*6, 0.0);
  unsigned offset = proc_id*6;
  Real * s_bounds = &spparks_bounds[0] + offset;
  s_bounds[0] = s_bounds[1] = s_bounds[2] = std::numeric_limits<Real>::max();
  s_bounds[3] = s_bounds[4] = s_bounds[5] = std::numeric_limits<Real>::min();
  for (std::set<SPPARKSID>::const_iterator i = unmatched_spparks.begin(); i != unmatched_spparks.end(); ++i)
  {
    s_bounds[0] = std::min(s_bounds[0], i->coor(0));
    s_bounds[1] = std::min(s_bounds[1], i->coor(1));
    s_bounds[2] = std::min(s_bounds[2], i->coor(2));
    s_bounds[3] = std::max(s_bounds[3], i->coor(0));
    s_bounds[4] = std::max(s_bounds[4], i->coor(1));
    s_bounds[5] = std::max(s_bounds[5], i->coor(2));
  }
  MeshTools::BoundingBox spparks_bb( Point(s_bounds[0], s_bounds[1], s_bounds[2]),
                                     Point(s_bounds[3], s_bounds[4], s_bounds[5]) );
  Parallel::sum(spparks_bounds);


  // B: Get ELK bounding boxes

  //
  // TODO: These bboxes could/should be based on non-matched elk nodes.
  //
  std::vector<Real> elk_bounds(num_procs*6, 0.0);
  Real * e_bounds = &elk_bounds[0] + offset;
  e_bounds[0] = e_bounds[1] = e_bounds[2] = std::numeric_limits<Real>::max();
  e_bounds[3] = e_bounds[4] = e_bounds[5] = std::numeric_limits<Real>::min();
  for (std::multiset<ELKID>::const_iterator i = elk_id.begin(); i != elk_id.end(); ++i)
  {
    e_bounds[0] = std::min(e_bounds[0], i->coor(0));
    e_bounds[1] = std::min(e_bounds[1], i->coor(1));
    e_bounds[2] = std::min(e_bounds[2], i->coor(2));
    e_bounds[3] = std::max(e_bounds[3], i->coor(0));
    e_bounds[4] = std::max(e_bounds[4], i->coor(1));
    e_bounds[5] = std::max(e_bounds[5], i->coor(2));
  }
  MeshTools::BoundingBox elk_bb( Point(e_bounds[0], e_bounds[1], e_bounds[2]),
                                 Point(e_bounds[3], e_bounds[4], e_bounds[5]) );
  Parallel::sum(elk_bounds);



  // C: Get number of processors that overlap my SPPARKS and ELK domains

  std::vector<unsigned> procs_overlapping_spparks_domain;
  std::vector<unsigned> procs_overlapping_elk_domain;
  for (unsigned i = 0; i < num_procs; ++i)
  {
    if ( i == proc_id )
    {
      continue;
    }
    offset = i * 6;
    e_bounds = &elk_bounds[0] + offset;
    MeshTools::BoundingBox e_box( Point(e_bounds[0], e_bounds[1], e_bounds[2]),
                                  Point(e_bounds[3], e_bounds[4], e_bounds[5]) );
    if (spparks_bb.intersect( e_box ))
    {
      procs_overlapping_spparks_domain.push_back( i );
    }

    //
    // ERROR?
    // Should this be s_bounds instead of e_bounds?
    //
    s_bounds = &spparks_bounds[0] + offset;
    MeshTools::BoundingBox s_box( Point(s_bounds[0], s_bounds[1], s_bounds[2]),
                                  Point(s_bounds[3], s_bounds[4], s_bounds[5]) );
    if (elk_bb.intersect( s_box ))
    {
      procs_overlapping_elk_domain.push_back( i );
    }
  }

  // D: Communicate number of ELK nodes

  std::vector<unsigned> num_elk_nodes(procs_overlapping_spparks_domain.size(), 0);

  MPI_Request recv_request[ std::max(procs_overlapping_spparks_domain.size(), procs_overlapping_elk_domain.size()) ];
  int comm_tag = 100;
  for (unsigned i = 0; i < procs_overlapping_spparks_domain.size(); ++i)
  {
    MPI_Irecv(&num_elk_nodes[i], 1, MPI_UNSIGNED, procs_overlapping_spparks_domain[i], comm_tag, libMesh::COMM_WORLD, &recv_request[i]);
  }

  for (unsigned i = 0; i < procs_overlapping_elk_domain.size(); ++i)
  {
    MPI_Send(&_num_local_elk_nodes, 1, MPI_UNSIGNED, procs_overlapping_elk_domain[i], comm_tag, libMesh::COMM_WORLD);
  }

  std::vector<MPI_Status> recv_status( std::max(procs_overlapping_spparks_domain.size(), procs_overlapping_elk_domain.size()) );
  MPI_Waitall(procs_overlapping_spparks_domain.size(), &recv_request[0], &recv_status[0]);


  // E: Communicate ELK nodes

  comm_tag = 200;
  int comm_tag_double = comm_tag + 1;
  const unsigned num_elk_nodes_total = std::accumulate(&num_elk_nodes[0], &num_elk_nodes[0]+num_elk_nodes.size(), 0);
  std::vector<libMesh::dof_id_type> remote_elk_nodes(num_elk_nodes_total);
  std::vector<Real> remote_elk_coords(num_elk_nodes_total*3);
  offset = 0;
  std::vector<MPI_Request> recv_request_coor(procs_overlapping_spparks_domain.size());
  for (unsigned i = 0; i < procs_overlapping_spparks_domain.size(); ++i)
  {
    MPI_Irecv(&remote_elk_nodes[offset], num_elk_nodes[i], MPI_UNSIGNED, procs_overlapping_spparks_domain[i], comm_tag, libMesh::COMM_WORLD, &recv_request[i]);
    MPI_Irecv(&remote_elk_coords[offset*3], num_elk_nodes[i]*3, MPI_DOUBLE, procs_overlapping_spparks_domain[i], comm_tag_double, libMesh::COMM_WORLD, &recv_request_coor[i]);
    offset += num_elk_nodes[i];
  }

  // Get vectors of ids, coordinates
  std::vector<libMesh::dof_id_type> elk_ids(_num_local_elk_nodes);
  std::vector<Real> elk_coords(3*_num_local_elk_nodes);
  offset = 0;
  for (std::multiset<ELKID>::const_iterator i = elk_id.begin(); i != elk_id.end(); ++i)
  {
    elk_ids[offset] = i->id;
    elk_coords[offset*3+0] = i->coor(0);
    elk_coords[offset*3+1] = i->coor(1);
    elk_coords[offset*3+2] = i->coor(2);
    ++offset;
  }

  offset = 0;
  for (unsigned i = 0; i < procs_overlapping_elk_domain.size(); ++i) {
    MPI_Send(&elk_ids[0], _num_local_elk_nodes, MPI_UNSIGNED, procs_overlapping_elk_domain[i], comm_tag, libMesh::COMM_WORLD);
    MPI_Send(&elk_coords[0], _num_local_elk_nodes*3, MPI_DOUBLE, procs_overlapping_elk_domain[i], comm_tag_double, libMesh::COMM_WORLD);
    ++offset;
  }

  MPI_Waitall(procs_overlapping_spparks_domain.size(), &recv_request[0], &recv_status[0]);
  MPI_Waitall(procs_overlapping_spparks_domain.size(), &recv_request_coor[0], &recv_status[0]);


  // F: Count matching nodes for each proc that sent ELK nodes

  comm_tag = 300;
  std::vector<unsigned> num_remote_matches( procs_overlapping_elk_domain.size() );
  offset = 0;
  for (unsigned i = 0; i < procs_overlapping_elk_domain.size(); ++i)
  {
    MPI_Irecv(&num_remote_matches[i], 1, MPI_UNSIGNED, procs_overlapping_elk_domain[i], comm_tag, libMesh::COMM_WORLD, &recv_request[i]);
    ++offset;
  }


  std::vector<std::vector<libMesh::dof_id_type> > matches(procs_overlapping_spparks_domain.size());
  std::vector<SPPARKSID> send_ids;
  std::vector<unsigned> send_procs;
  offset = 0;
  for (unsigned i = 0; i < procs_overlapping_spparks_domain.size(); ++i)
  {
    for (unsigned j = 0; j < num_elk_nodes[i]; ++j)
    {
      SPPARKSID tmp( remote_elk_nodes[offset],
                     Point(remote_elk_coords[offset*3+0],
                           remote_elk_coords[offset*3+1],
                           remote_elk_coords[offset*3+2]) );
      std::set<SPPARKSID>::iterator iter = spparks_id.find( tmp );
      if ( iter != spparks_id.end() )
      {
        matches[i].push_back( tmp.index );
        send_ids.push_back( *iter );
        send_procs.push_back( procs_overlapping_spparks_domain[i] );

        unsigned index = iter->index;
        Point p(xyz_array[index][0],
                _dim > 1 ? xyz_array[index][1] : 0,
                _dim > 2 ? xyz_array[index][2] : 0);

        _spparks_to_proc[SPPARKSID(iter->index, p)].push_back( procs_overlapping_spparks_domain[i] );
      }
      ++offset;
    }
  }


  std::vector<unsigned> sizes(procs_overlapping_spparks_domain.size());
  for (unsigned i = 0; i < procs_overlapping_spparks_domain.size(); ++i)
  {
    sizes[i] = matches[i].size();
    MPI_Send(&sizes[i], 1, MPI_UNSIGNED, procs_overlapping_spparks_domain[i], comm_tag, libMesh::COMM_WORLD);
  }

  MPI_Waitall(procs_overlapping_elk_domain.size(), &recv_request[0], &recv_status[0]);


  // G: Communicate matching nodes

  comm_tag = 400;
  std::vector<std::vector<unsigned> > matched_elk_ids(procs_overlapping_elk_domain.size());
  for (unsigned i = 0; i < procs_overlapping_elk_domain.size(); ++i)
  {
    matched_elk_ids[i].resize( num_remote_matches[i] );
    MPI_Irecv(&matched_elk_ids[i][0], num_remote_matches[i], MPI_UNSIGNED, procs_overlapping_elk_domain[i], comm_tag, libMesh::COMM_WORLD, &recv_request[i]);
  }

  for (unsigned i = 0; i < procs_overlapping_spparks_domain.size(); ++i)
  {
    MPI_Send(&matches[i][0], sizes[i], MPI_UNSIGNED, procs_overlapping_spparks_domain[i], comm_tag, libMesh::COMM_WORLD);
  }

  MPI_Waitall(procs_overlapping_elk_domain.size(), &recv_request[0], &recv_status[0]);


  // H: Generate final recv communication map

  for (unsigned i = 0; i < procs_overlapping_elk_domain.size(); ++i)
  {
    for (unsigned j = 0; j < num_remote_matches[i]; ++j)
    {
      _sending_proc_to_elk_id[procs_overlapping_elk_domain[i]].push_back( matched_elk_ids[i][j] );
    }
  }


}
