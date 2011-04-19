// Moose
#include "MooseMesh.h"

// libMesh
#include "boundary_info.h"
#include "elem.h"
#include "libmesh_config.h"
#include "libmesh_common.h"
#include "libmesh_logging.h"
#include "location_maps.h"
#include "mesh_base.h"
#include "mesh_communication.h"
#include "mesh_tools.h"
#include "parallel.h"
#include "parallel_mesh.h"
#include "parallel_ghost_sync.h"
#include "utility.h"
#include "remote_elem.h"

namespace Moose
{

/**
 * Specific weak ordering for Elem*'s to be used in a set.
 * We use the id, but first sort by level.  This guarantees 
 * when traversing the set from beginning to end the lower 
 * level (parent) elements are encountered first.
 */
struct CompareElemIdsByLevel
{
  bool operator()(const Elem *a,
                  const Elem *b) const
    {
      libmesh_assert (a);
      libmesh_assert (b);
      const unsigned int
	al = a->level(), bl = b->level(),
	aid = a->id(),   bid = b->id();

      return (al == bl) ? aid < bid : al < bl;
    }
};

/**
 * Bitmask for which faces have boundary conditions.
 */
const unsigned int 
FaceNeighborMask[] = { 1,    // face 0 neighbor
                       2,    // face 1 neighbor
                       4,    // face 2 neighbor
                       8,    // face 3 neighbor
                       16,   // face 4 neighbor
                       32 }; // face 5 neighbor


void gatherNearbyElements (MooseMesh & moose_mesh, std::set<unsigned int> boundaries_to_ghost)
{

  ParallelMesh & mesh = libmesh_cast_ref<ParallelMesh&>(moose_mesh.getMesh());
  
  // Don't need to do anything if there is
  // only one processor.
  if (libMesh::n_processors() == 1)
    return;  

  //------------------------------------------------------------------
  // The purpose of this function is to provide neighbor data structure
  // consistency for a parallel, distributed mesh.  In libMesh we require
  // that each local element have access to a full set of valid face
  // neighbors.  In some cases this requires us to store "ghost elements" -
  // elements that belong to other processors but we store to provide
  // data structure consistency.  Also, it is assumed that any element
  // with a NULL neighbor resides on a physical domain boundary.  So,
  // even our "ghost elements" must have non-NULL neighbors.  To handle
  // this the concept of "RemoteElem" is used - a special construct which
  // is used to denote that an element has a face neighbor, but we do
  // not actually store detailed information about that neighbor.  This
  // is required to prevent data structure explosion.
  //
  // So when this method is called we should have only local elements.
  // These local elements will then find neighbors among the local
  // element set.  After this is completed, any element with a NULL
  // neighbor has either (i) a face on the physical boundary of the mesh,
  // or (ii) a neighboring element which lives on a remote processor.
  // To handle case (ii), we communicate the global node indices connected
  // to all such faces to our neighboring processors.  They then send us
  // all their elements with a NULL neighbor that are connected to any
  // of the nodes in our list.
  //------------------------------------------------------------------

  
  // Let's begin with finding consistent neighbor data information
  // for all the elements we currently have.  We'll use a clean
  // slate here - clear any existing information, including RemoteElem's.
//  mesh.find_neighbors (/* reset_remote_elements = */ true,
//		       /* reset_current_list    = */ true);    

  std::vector<unsigned int> el;
  std::vector<unsigned short int> sl;
  std::vector<short int> il;

  moose_mesh.build_side_list(el, sl, il);

  // register a derived datatype to use in shipping nodes  
  Parallel::DataType packed_node_datatype = Node::PackedNode::create_mpi_datatype();
  packed_node_datatype.commit();

  // Get a few unique message tags to use in communications; we'll
  // default to some numbers around pi*10000
  Parallel::MessageTag
    node_list_tag         = Parallel::Communicator_World.get_unique_tag(31415),
    element_neighbors_tag = Parallel::Communicator_World.get_unique_tag(31416);

  // A list of all the processors which *may* contain neighboring elements.
  // (for development simplicity, just make this the identity map)
  std::vector<unsigned int> adjacent_processors;
  for (unsigned int pid=0; pid<libMesh::n_processors(); pid++)
    if (pid != libMesh::processor_id())
      adjacent_processors.push_back (pid);


  const unsigned int n_adjacent_processors = adjacent_processors.size();

  //-------------------------------------------------------------------------
  // Let's build a list of all nodes which live on NULL-neighbor sides.
  // For simplicity, we will use a set to build the list, then transfer
  // it to a vector for communication.
  std::vector<unsigned int> my_interface_node_list;
  std::vector<const Elem*>  my_interface_element_list;  
  {
    std::set<unsigned int> my_interface_node_set;
    std::set<const Elem*> my_interface_element_set;

    /*
    // since parent nodes are a subset of children nodes, this should be sufficient
    MeshBase::const_element_iterator       it     = mesh.active_local_elements_begin();
    const MeshBase::const_element_iterator it_end = mesh.active_local_elements_end();

    for (; it != it_end; ++it)
      {
	const Elem * const elem = *it;
	libmesh_assert (elem != NULL);

	if (moose_mesh.side_with_boundary_id(elem, 2) != libMesh::invalid_uint
            || moose_mesh.side_with_boundary_id(elem, 3) != libMesh::invalid_uint)
	  {
	    my_interface_elements.push_back(elem); // add the element, but only once, even
	                                           // if there are multiple NULL neighbors
            
	    for (unsigned int s=0; s<elem->n_sides(); s++)
	      if (elem->neighbor(s) == NULL)
		{
		  AutoPtr<Elem> side(elem->build_side(s));
		  
		  for (unsigned int n=0; n<side->n_vertices(); n++)
		    my_interface_node_set.insert (side->node(n));   
		}
	  }

      }
    */

    unsigned int num_elem = el.size();

    for(unsigned int i=0; i < num_elem; i++)
    {
      // See if this boundary entry is in the list
      if(boundaries_to_ghost.find(il[i]) != boundaries_to_ghost.end())
      {
        const Elem * const elem = mesh.elem(el[i]);
        my_interface_element_set.insert(elem);
        
        AutoPtr<Elem> side(elem->build_side(sl[i]));
            
        for (unsigned int n=0; n<side->n_vertices(); n++)
          my_interface_node_set.insert (side->node(n));
      }
    }    

    my_interface_element_list.reserve (my_interface_element_set.size());
    my_interface_element_list.insert  (my_interface_element_list.end(),
                                       my_interface_element_set.begin(),
                                       my_interface_element_set.end());

    my_interface_node_list.reserve (my_interface_node_set.size());
    my_interface_node_list.insert  (my_interface_node_list.end(),
				    my_interface_node_set.begin(),
				    my_interface_node_set.end());

  }
  
  if (true)
    libMesh::out << "[" << libMesh::processor_id() << "] "
	          << "mesh.n_nodes()=" << mesh.n_nodes() << ", "
	          << "my_interface_node_list.size()=" << my_interface_node_list.size()
	          << std::endl;
  
  
  // we will now send my_interface_node_list to all of the adjacent processors.
  // note that for the time being we will copy the list to a unique buffer for 
  // each processor so that we can use a nonblocking send and not access the
  // buffer again until the send completes.  it is my understanding that the
  // MPI 2.1 standard seeks to remove this restriction as unnecessary, so in
  // the future we should change this to send the same buffer to each of the
  // adjacent processors. - BSK 11/17/2008
  std::vector<std::vector<unsigned int> > 
    my_interface_node_xfer_buffers (n_adjacent_processors, my_interface_node_list);
  std::vector<Parallel::Request> my_interface_node_list_requests (n_adjacent_processors);
  std::map<unsigned int, unsigned int> n_comm_steps;

  for (unsigned int comm_step=0; comm_step<n_adjacent_processors; comm_step++)
    {
      n_comm_steps[adjacent_processors[comm_step]]=1;
      Parallel::send (adjacent_processors[comm_step],
		      my_interface_node_xfer_buffers[comm_step],
		      my_interface_node_list_requests[comm_step],
		      node_list_tag);
    }

  //-------------------------------------------------------------------------
  // processor pairings are symmetric - I expect to receive an interface node
  // list from each processor in adjacent_processors as well!
  // now we will catch an incoming node list for each of our adjacent processors.
  //
  // we are done with the adjacent_processors list - note that it is in general
  // a superset of the processors we truly share elements with.  so let's
  // clear the superset list, and we will fill it with the true list.
  adjacent_processors.clear();
  
  std::vector<unsigned int> common_interface_node_list;

  // send buffers.  we will fill these with data from the elements we own 
  // which share nodes with an adjacent processor.  we will slightly abuse 
  // the node_bcs_sent and element_bcs_sent buffers - we'll fill them and then tack
  // them on to the end of the elements_sent buffer to reduce the message count.
  std::vector<std::vector<Node::PackedNode> > nodes_sent(n_adjacent_processors);
  std::vector<std::vector<int> > elements_sent(n_adjacent_processors);
  std::vector<int> node_bcs_sent, &element_bcs_sent(node_bcs_sent);
  std::vector<std::vector<unsigned int> > element_neighbors_sent(n_adjacent_processors);
    
  std::vector<Parallel::Request> 
    node_send_requests(n_adjacent_processors), 
    element_send_requests(n_adjacent_processors),
    element_neighbor_send_requests(n_adjacent_processors);

  // receive buffers
  std::vector<Node::PackedNode> nodes_received;
  std::vector<int> elements_received;
  std::vector<unsigned int> element_neighbors_received;

  // we expect two classess of messages - 
  // (1) incoming interface node lists, to which we will reply with our elements 
  //     touching nodes in the list, and
  // (2) replies from the requests we sent off previously.  
  //  (2.a) - nodes
  //  (2.b) - element connectivity & bc info
  // so we expect 3 communications from each adjacent processor.
  // by structuring the communication in this way we hopefully impose no 
  // order on the handling of the arriving messages.  in particular, we
  // should be able to handle the case where we receive a request and
  // all replies from processor A before even receiving a request from
  // processor B.
  unsigned int 
    n_node_replies_sent=0, n_node_replies_received=0, 
    n_elem_replies_sent=0, n_elem_replies_received=0;

  for (unsigned int comm_step=0; comm_step<3*n_adjacent_processors; comm_step++)
    {
      //------------------------------------------------------------------
      // catch incoming node list
      Parallel::Status
	status(Parallel::probe (Parallel::any_source,
				node_list_tag));      
      const unsigned int
	source_pid_idx = status.source(),
	dest_pid_idx   = source_pid_idx;

      //------------------------------------------------------------------
      // first time - incoming request
      if (n_comm_steps[source_pid_idx] == 1)
	{
	  n_comm_steps[source_pid_idx]++;
	  
	  Parallel::receive (source_pid_idx,
			     common_interface_node_list,
			     node_list_tag);
	  const unsigned int	
	    their_interface_node_list_size = common_interface_node_list.size();

          std::cerr<<"their_interface_node_list_size: "<<their_interface_node_list_size<<std::endl;
          
	  
	  // we now have the interface node list from processor source_pid_idx.
	  // now we can find all of our elements which touch any of these nodes
	  // and send copies back to this processor.  however, we can make our
	  // search more efficient by first excluding all the nodes in
	  // their list which are not also contained in
	  // my_interface_node_list.  we can do this in place as a set
	  // intersection.

          /*  DRG: Later we should do a check here to see if the incoming nodes are geometrically close to our interface nodes....
	  common_interface_node_list.erase
	    (std::set_intersection (my_interface_node_list.begin(),
				    my_interface_node_list.end(),
				    common_interface_node_list.begin(),
				    common_interface_node_list.end(),
				    common_interface_node_list.begin()),
	     common_interface_node_list.end());
          */
	  
	  if (true)
	    libMesh::out << "[" << libMesh::processor_id() << "] "
		          << "my_interface_node_list.size()="       << my_interface_node_list.size()
		          << ", [" << source_pid_idx << "] "
		          << "their_interface_node_list.size()="    << their_interface_node_list_size
		          << ", common_interface_node_list.size()=" << common_interface_node_list.size()
		          << std::endl;

          std::cerr<<libMesh::processor_id()<<": common_inl: "<<common_interface_node_list.size()<<std::endl;

	  // Check for quick return?
	  if (common_interface_node_list.empty())
	    {
	      // let's try to be smart here - if we have no nodes in common,
	      // we cannot share elements.  so post the messages expected
	      // from us here and go on about our business.  
	      // note that even though these are nonblocking sends
	      // they should complete essentially instantly, because
	      // in all cases the send buffers are empty
	      Parallel::send (dest_pid_idx,
			      nodes_sent[n_node_replies_sent],
			      packed_node_datatype,
			      node_send_requests[n_node_replies_sent],
			      node_list_tag);
	      n_node_replies_sent++;

	      Parallel::send (dest_pid_idx,
			      elements_sent[n_elem_replies_sent],
			      element_send_requests[n_elem_replies_sent],
			      node_list_tag);

	      Parallel::send (dest_pid_idx,
			      element_neighbors_sent[n_elem_replies_sent],
			      element_neighbor_send_requests[n_elem_replies_sent],
			      element_neighbors_tag);
	      n_elem_replies_sent++;

	      continue;
	    }
	  // otherwise, this really *is* an adjacent processor.
	  adjacent_processors.push_back(source_pid_idx);

          std::cerr<<libMesh::processor_id()<<": Added adjacent processor!"<<std::endl;


	  // Now we need to see which of our elements touch the nodes in the list.
	  // We built a reduced element list above, and we know the pointers are
	  // not NULL.
	  // We will certainly send all the active elements which intersect source_pid_idx,
	  // but we will also ship off the other elements in the same family tree
	  // as the active ones for data structure consistency.  We also
	  // ship any nodes connected to these elements.  Note some of these nodes
	  // and elements may be replicated from other processors, but that is OK.
	  std::set<const Elem*, CompareElemIdsByLevel> elements_to_send;
	  std::set<const Node*> connected_nodes;
	  std::vector<const Elem*> family_tree;
	  
	  for (unsigned int e=0, n_shared_nodes=0; e<my_interface_element_list.size(); e++, n_shared_nodes=0)
	    {
	      const Elem * elem = my_interface_element_list[e];
	      /*
	      for (unsigned int n=0; n<elem->n_vertices(); n++)
		if (std::binary_search (common_interface_node_list.begin(),
					common_interface_node_list.end(),
					elem->node(n)))
		  { 
		    n_shared_nodes++;
		    
		    // TBD - how many nodes do we need to share 
		    // before we care?  certainly 2, but 1?  not 
		    // sure, so let's play it safe...
		    if (n_shared_nodes > 0) break;
                    }*/
	      
	      if (true)//n_shared_nodes) // share at least one node?
		{
		  elem = elem->top_parent();
		  
		  // avoid a lot of duplicated effort -- if we already have elem
		  // in the set its entire family tree is already in the set.
		  if (!elements_to_send.count(elem))
		    {
#ifdef LIBMESH_ENABLE_AMR
		      elem->family_tree(family_tree);
#else
		      family_tree.clear();
		      family_tree.push_back(elem);
#endif
		      for (unsigned int leaf=0; leaf<family_tree.size(); leaf++)
			{
			  elem = family_tree[leaf];
			  elements_to_send.insert (elem);
                          
                          std::cerr<<libMesh::processor_id()<<": Sending elem: "<<elem->id()<<std::endl;
                      
			  
			  for (unsigned int n=0; n<elem->n_nodes(); n++)
			    connected_nodes.insert (elem->get_node(n));		  
			}		    
		    }
		}
	    }
	  
	  // The elements_to_send and connected_nodes sets now contain all
	  // the elements and nodes we need to send to this processor.
	  // All that remains is to pack up the objects (along with
	  // any boundary conditions) and send the messages off.
	  {
	    if (elements_to_send.empty()) libmesh_assert (connected_nodes.empty());
	    if (connected_nodes.empty())  libmesh_assert (elements_to_send.empty());
	    
	    const unsigned int n_nodes_sent = connected_nodes.size();
	    nodes_sent[n_node_replies_sent].reserve (n_nodes_sent);
	    node_bcs_sent.clear();
	
	    for (std::set<const Node*>::const_iterator node_it = connected_nodes.begin();
		 node_it != connected_nodes.end(); ++node_it)
	      {
		nodes_sent[n_node_replies_sent].push_back (Node::PackedNode(**node_it));
		
		// add the node if it has BCs
                std::vector<short int> bcs = mesh.boundary_info->boundary_ids(*node_it);
                
		if (!bcs.empty())
		  {
		    node_bcs_sent.push_back((*node_it)->id());
		    node_bcs_sent.push_back(bcs.size());
                    for(unsigned int bc_it=0; bc_it < bcs.size(); bc_it++)
                      node_bcs_sent.push_back(bcs[bc_it]);
		  }
	      }
	    connected_nodes.clear();
	
	    // send the nodes off to the destination processor
	    Parallel::send (dest_pid_idx,
			    nodes_sent[n_node_replies_sent],
			    packed_node_datatype,
			    node_send_requests[n_node_replies_sent],
			    node_list_tag);
	    n_node_replies_sent++;
	
	    // let's pack the node bc data in the front of the element
	    // connectivity buffer
	    const unsigned int
	      n_node_bcs_sent = node_bcs_sent.size();
	    
	    elements_sent[n_elem_replies_sent].clear();
	    elements_sent[n_elem_replies_sent].insert (elements_sent[n_elem_replies_sent].end(),
						       node_bcs_sent.begin(),
						       node_bcs_sent.end());
	
	    // this is really just a reference to node_bcs_sent,
	    // so be careful not to clear it before packing the
	    // node_bcs_sent into the elements_sent buffer!!
	    const unsigned int n_elements_sent = elements_to_send.size();
	    element_neighbors_sent[n_elem_replies_sent].reserve(2*n_elements_sent+1);
	    element_bcs_sent.clear(); 
	    
	    for (std::set<const Elem*, CompareElemIdsByLevel>::const_iterator 
		   elem_it = elements_to_send.begin(); elem_it != elements_to_send.end(); ++elem_it)
	      {
		Elem::PackedElem::pack (elements_sent[n_elem_replies_sent], *elem_it);
		
		// let's put the element index in the element_neighbors_sent
		// buffer - we will later overwrite it with a bitmask of the
		// element neighbor information to send out.  this will be used 
		// to create RemoteElems where appropriate.
		element_neighbors_sent[n_elem_replies_sent].push_back((*elem_it)->id());
		element_neighbors_sent[n_elem_replies_sent].push_back(0);
		
		// if this is a level-0 element look for boundary conditions
		if ((*elem_it)->level() == 0)
		  for (unsigned int s=0; s<(*elem_it)->n_sides(); s++)
		    if ((*elem_it)->neighbor(s) == NULL)
                      {
                        const std::vector<short int>& bc_ids = mesh.boundary_info->boundary_ids(*elem_it, s);
                        for (std::vector<short int>::const_iterator id_it=bc_ids.begin(); id_it!=bc_ids.end(); ++id_it)
                          {
                            const short int bc_id = *id_it;
		            if (bc_id != mesh.boundary_info->invalid_id)
			      {
			        element_bcs_sent.push_back ((*elem_it)->id());
			        element_bcs_sent.push_back (s);
			        element_bcs_sent.push_back (bc_id);
			      }
                          }
                      }
	      }
	    element_neighbors_sent[n_elem_replies_sent].push_back (dest_pid_idx);
	    elements_to_send.clear();
	    
	    // let's pack the node bc data in the front of the element
	    // connectivity buffer
	    const unsigned int
	      n_elem_bcs_sent = element_bcs_sent.size() / 3;
	    
	    elements_sent[n_elem_replies_sent].insert (elements_sent[n_elem_replies_sent].end(),
						       element_bcs_sent.begin(),
						       element_bcs_sent.end());
	    
	    // only append to the message if it is not empty
	    if (!elements_sent[n_elem_replies_sent].empty())
	      {
		// let's tack three ints on to
		// the end of the elements_sent
		// buffer for use on the receiving end
		elements_sent[n_elem_replies_sent].push_back (n_elements_sent);
		elements_sent[n_elem_replies_sent].push_back (n_elem_bcs_sent);
		elements_sent[n_elem_replies_sent].push_back (n_node_bcs_sent);
	      }
	    
	    // send the elements off to the destination processor
	    Parallel::send (dest_pid_idx,
			    elements_sent[n_elem_replies_sent],
			    element_send_requests[n_elem_replies_sent],
			    node_list_tag);
	    n_elem_replies_sent++;
	  }
	}



      //------------------------------------------------------------------
      // second time - reply of nodes
      else if (n_comm_steps[source_pid_idx] == 2)
	{
	  n_comm_steps[source_pid_idx]++;
	
	  Parallel::receive (source_pid_idx,
			     nodes_received,
			     packed_node_datatype,
			     node_list_tag);
	  n_node_replies_received++;
	  
	  // add the nodes we just received
	  for (unsigned int n=0; n<nodes_received.size(); n++)
	    mesh.insert_node (nodes_received[n].build_node().release());
	}



      //------------------------------------------------------------------
      // third time - elements & bcs
      else if (n_comm_steps[source_pid_idx] == 3)
	{ 
	  n_comm_steps[source_pid_idx]++;

	  Parallel::receive (source_pid_idx,
			     elements_received,
			     node_list_tag);	  
	  n_elem_replies_received++;

          std::cerr<<libMesh::processor_id()<<": Received elems: "<<elements_received.size()<<std::endl;          
	  
	  if (elements_received.empty())
	    continue;
	  
	  libmesh_assert (elements_received.size() > 3);
	  
	  ////////////////////////////////////////////////////////////////////////////////
	  // remember: elements_sent = 
	  //          { [node bd data] [element conntectivity] [element bc data] # # # }
	  ////////////////////////////////////////////////////////////////////////////////
	  const unsigned int n_node_bcs_received = elements_received.back(); elements_received.pop_back();
	  const unsigned int n_elem_bcs_received = elements_received.back(); elements_received.pop_back();
	  const unsigned int n_elements_received = elements_received.back(); elements_received.pop_back();
	  
	  // counter into the bc/element connectivty buffer
	  unsigned int cnt=0;
	  
	  // add any node bcs
	  while (cnt < n_node_bcs_received)
	    {
              const unsigned int node_id = elements_received[cnt++];
              const unsigned int num_bcs = elements_received[cnt++];

              libmesh_assert (mesh.node_ptr(node_id));
            
              for(unsigned int bc_it=0; bc_it < num_bcs; bc_it++)
                mesh.boundary_info->add_node (mesh.node_ptr(node_id), elements_received[cnt++]);
	    }
      
	  // add the elements we just received
	  for (unsigned int e=0; e<n_elements_received; e++)
	    {
	      // Unpack the element 
	      Elem::PackedElem packed_elem (elements_received.begin()+cnt);
	  
	      // The ParallelMesh::elem(i) member will return NULL if the element
	      // is not in the mesh.  We rely on that here, so it better not change!
	      Elem *elem = mesh.elem(packed_elem.id());

                std::cerr<<libMesh::processor_id()<<": packed elem: "<<packed_elem.id()<<std::endl;          
	  
	      // if we already have this element, make sure its properties match
	      // but then go on
	      if (elem)
		{
		  libmesh_assert (elem->level()             == packed_elem.level());
		  libmesh_assert (elem->id()                == packed_elem.id());
		  libmesh_assert (elem->processor_id()      == packed_elem.processor_id());
		  libmesh_assert (elem->subdomain_id()      == packed_elem.subdomain_id());
		  libmesh_assert (elem->type()              == packed_elem.type());
#ifdef LIBMESH_ENABLE_AMR
		  libmesh_assert (elem->p_level()           == packed_elem.p_level());
		  libmesh_assert (elem->refinement_flag()   == packed_elem.refinement_flag());
		  libmesh_assert (elem->p_refinement_flag() == packed_elem.p_refinement_flag());
		  
		  if (elem->level() > 0)
		    {
		      libmesh_assert (elem->parent()->id() == static_cast<unsigned int>(packed_elem.parent_id()));
		      libmesh_assert (elem->parent()->child(packed_elem.which_child_am_i()) == elem);
		    }	      
#endif
		  libmesh_assert (elem->n_nodes() == packed_elem.n_nodes());
		}
	      else
		{
		  // We need to add the element.
#ifdef LIBMESH_ENABLE_AMR
		  // maybe find the parent
		  if (packed_elem.level() > 0)
		    {
		      Elem *parent = mesh.elem(packed_elem.parent_id());
		      
		      // Note that we were very careful to construct the send connectivity
		      // so that parents are encountered before children.  So if we get here
		      // and can't find the parent that is a fatal error.
		      if (parent == NULL)
			{
			  libMesh::err << "Parent element with ID " << packed_elem.parent_id()
				        << " not found." << std::endl; 
			  libmesh_error();
			}
		      
		      elem = packed_elem.unpack (mesh, parent);
		    }
		  else
		    {
		      libmesh_assert (packed_elem.parent_id() == -1);
		      elem = packed_elem.unpack (mesh);
		    }
#else // !defined(LIBMESH_ENABLE_AMR)
		  elem = packed_elem.unpack (mesh);
#endif		
		  // Good to go.  Add to the mesh.
		  libmesh_assert (elem);
		  libmesh_assert (elem->n_nodes() == packed_elem.n_nodes());

                  // Make sure we insert it using insert_extra_ghost_elem()
                  // This will keep it from being deleted by delete_remote_elements()
		  mesh.insert_extra_ghost_elem(elem);

                  std::cerr<<libMesh::processor_id()<<": inserted elem: "<<elem->id()<<std::endl;
		}
	  
	      // properly position cnt for the next element 
	      cnt += Elem::PackedElem::header_size + packed_elem.n_nodes();	      
	    } // done adding elements
      
	  // add any element bcs
	  unsigned int n_elem_bcs=0;
	  while (cnt < elements_received.size())
	    {
	      n_elem_bcs++;
	      const unsigned int elem_id = elements_received[cnt++];
	      const unsigned int side    = elements_received[cnt++];
	      const int bc_id            = elements_received[cnt++];
	      
	      libmesh_assert (mesh.elem(elem_id));
	      mesh.boundary_info->add_side (mesh.elem(elem_id), side, bc_id);
	    }
	  libmesh_assert (n_elem_bcs_received == n_elem_bcs);
	}

      // Huh?
      else
	{
	  libMesh::err << "ERROR:  unexpected number of replies: "
		        << n_comm_steps[source_pid_idx]
		    << std::endl;
	}
    } // done catching & processing replies associated with tag ~ 100,000pi
  
  // Update neighbor information with the new elements,
  // but don't throw away old information.
  mesh.find_neighbors (/* reset_remote_elements = */ true,
		       /* reset_current_list    = */ false);    
  
  // OK, now at least all local elements should have a full set
  // of neighbor information.  we still need to resolve remote
  // element neighbor information though. to do this, we need
  // to get the neighbor information from the processors that
  // sent elements to us.
  // 
  // first, send element face information to requisite processors
  for (unsigned int comm_step=0; comm_step<n_adjacent_processors; comm_step++)
    if (!element_neighbors_sent[comm_step].empty()) // if it is empty we already sent it!
      {
	const unsigned int dest_pid_idx = 
	  element_neighbors_sent[comm_step].back();
	element_neighbors_sent[comm_step].pop_back();
	
	for (unsigned int pos=0; pos<element_neighbors_sent[comm_step].size(); pos+=2)
	  {
	    const unsigned int elem_global_idx = element_neighbors_sent[comm_step][pos+0];
	    const Elem *elem = mesh.elem(elem_global_idx);
	    unsigned int face_neighbor_bitmask = 0;
	    
	    for (unsigned int s=0; s<elem->n_sides(); s++)
	      if (elem->neighbor(s) != NULL)
		face_neighbor_bitmask |= FaceNeighborMask[s];
	    
	    element_neighbors_sent[comm_step][pos+1] = face_neighbor_bitmask;
	  }
      
	Parallel::send (dest_pid_idx,
			element_neighbors_sent[comm_step],
			element_neighbor_send_requests[comm_step],
			element_neighbors_tag);
      } // done sending neighbor information

  // second, receive and process all face neighbor information
  // from the processors which sent us elements
  for (unsigned int comm_step=0; comm_step<n_elem_replies_received; comm_step++)
    {
      Parallel::receive (Parallel::any_source,
			 element_neighbors_received,
			 element_neighbors_tag);

      for (unsigned int pos=0; pos<element_neighbors_received.size(); pos+=2)
	{
	  const unsigned int 
	    elem_global_idx       = element_neighbors_received[pos+0],
	    face_neighbor_bitmask = element_neighbors_received[pos+1];

	  Elem *elem = mesh.elem(elem_global_idx);

	  for (unsigned int s=0; s<elem->n_sides(); s++)
	    if ((face_neighbor_bitmask & FaceNeighborMask[s]) // if there is a face neighbor...
		&& (elem->neighbor(s) == NULL))               // ...and we do not have it
	      elem->set_neighbor(s,const_cast<RemoteElem*>(remote_elem)); // then it is a remote element
	}      
    } // done catching & processing neighbor information

  // allow any pending requests to complete
  Parallel::wait (my_interface_node_list_requests);
  Parallel::wait (node_send_requests);
  Parallel::wait (element_send_requests);
  Parallel::wait (element_neighbor_send_requests);  

  // unregister MPI datatypes
  packed_node_datatype.free();
}

}
