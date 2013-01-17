#include "NodalArea.h"

#include <numeric>

template<>
InputParameters validParams<NodalArea>()
{
  InputParameters params = validParams<SideIntegralVariableUserObject>();

  params.set<MooseEnum>("execute_on") = "timestep_begin";
  params.set<MooseEnum>("execute_on") = "residual";
  return params;
}



NodalArea::NodalArea(const std::string & name, InputParameters parameters) :
    SideIntegralVariableUserObject(name, parameters),
    _resetCommunication(true),
    _phi( _var.phiFace() )
{}

NodalArea::~NodalArea()
{}

void
NodalArea::threadJoin(const UserObject & fred)
{
  const NodalArea & na = dynamic_cast<const NodalArea &>(fred);

  std::map<unsigned, Real>::const_iterator it = na._node_areas.begin();
  for ( ; it != _node_areas.end(); ++it )
  {
    _node_areas[it->first] += it->second;
  }
}

Real
NodalArea::computeQpIntegral()
{
  return 1;
}

void
NodalArea::initialize()
{
  _node_areas.clear();
}

void
NodalArea::execute()
{
  std::vector<Real> nodeAreas(_phi.size());
  for ( unsigned qp(0); qp < _qrule->n_points(); ++qp )
  {
    for ( unsigned j(0); j < _phi.size(); ++j )
    {
      nodeAreas[j] +=  (_phi[j][qp] * _JxW[qp] * _coord[qp]);
    }
  }
  for ( unsigned j(0); j < _phi.size(); ++j )
  {
    const Real area = nodeAreas[j];
    if (area != 0)
    {
      _node_areas[_current_elem->node(j)] += area;
    }
  }
}

void
NodalArea::finalize()
{
  if (_resetCommunication)
  {
    initializeCommunication();
    _resetCommunication = false;
  }

  communicate();
}

Real
NodalArea::nodalArea( unsigned id ) const
{
  std::map<unsigned, Real>::const_iterator it = _node_areas.find( id );
  Real retVal(0);
  if (it != _node_areas.end())
  {
    retVal = it->second;
  }
  return retVal;
}



void
NodalArea::initializeCommunication()
{
  const unsigned numProc = libMesh::n_processors();
  const unsigned myProc = libMesh::processor_id();

  // Get the number of nodes on each processor
  std::vector<unsigned> numNodesVec(numProc, 0);
  numNodesVec[myProc] = _node_areas.size();
  Parallel::sum(numNodesVec);

  // Get the total number of nodes and the location where this proc's nodes will
  //   be inserted into the vector of all nodes
  const unsigned totalNumNodes = std::accumulate(numNodesVec.begin(), numNodesVec.end(), 0);
  unsigned index = std::accumulate(&numNodesVec[0], &numNodesVec[0]+myProc, 0);

  // Fill vector with this proc's nodes
  std::vector<unsigned> nodesVec(totalNumNodes, 0);
  for ( std::map<unsigned, Real>::iterator it = _node_areas.begin(); it != _node_areas.end(); ++it )
  {
    nodesVec[index++] = it->first;
  }

  // Get list of all nodes
  Parallel::sum(nodesVec);

  // Collect nodes, counting how many times each appears
  std::map<unsigned, unsigned> countMap;
  for ( unsigned i(0); i < totalNumNodes; ++i )
  {
    ++countMap[nodesVec[i]];
  }

  // Record number of communicating nodes and the index of each
  //   communicating node on this processor
  _commMap.clear();

  unsigned counter(0);
  for ( std::map<unsigned, unsigned>::iterator it = countMap.begin(); it != countMap.end(); ++it )
  {
    if (it->second > 1) // This node communicates
    {
      if (_node_areas.find( it->first ) != _node_areas.end()) // This proc has this node
      {
        // Global id points to index in vector (global to local)
        _commMap[it->first] = counter;
      }
      ++counter;
    }
  }

  _commVec.resize(counter);
}

void
NodalArea::communicate()
{
  _commVec.assign(_commVec.size(), 0);
  for ( std::map<unsigned,unsigned>::iterator it = _commMap.begin(); it != _commMap.end(); ++it )
  {
    _commVec[it->second] = _node_areas[it->first];
  }
  Parallel::sum(_commVec);
  for ( std::map<unsigned,unsigned>::iterator it = _commMap.begin(); it != _commMap.end(); ++it )
  {
    _node_areas[it->first] = _commVec[it->second];
  }
}
