import numpy as np
import sys

def read_data(fname, tag_start, tag_end):
  start_record = False
  end_record = False
  data = []
  with open(fname) as fp:
    for cnt, line in enumerate(fp):
      if line.strip() == tag_end:
        end_record = True
      if start_record and not end_record:
        data.append(np.fromstring(line.strip(), dtype=int, sep=' ' ))
      if line.strip() == tag_start:
        start_record = True

  d =  {words[0]:words[1:] for words in data}

  return d

def compute_nnode(elem_node):
  n = len(elem_node)
  nodes = []
  for i in range(n):
    data = elem_node[i]
    for key, val in data.items():
      for v in val:
        nodes.append(v)

  nodes = np.array(nodes)
  nnodes = len(np.unique(nodes))
  print("the mesh has {} nodes".format(nnodes))

def node_to_elem_map(elem_node):

  node_to_elems = {}
  node_to_elems_sorted = {}

  for e, nodes in elem_node.items():
    for node in nodes:
      if node not in node_to_elems.keys():
        node_to_elems[node] = set()
      node_to_elems[node].add(e)


  for node, elems in node_to_elems.items():
    node_to_elems_sorted[node]= sorted(elems)

  return node_to_elems_sorted

def nodeDomainsCPU(nodes_to_elem, elem_to_subdomain, elem_pid):

  nodes_domains = {}
  nodes_cpu = {}
  nodes_multiplicity = {}
  for node, elems in nodes_to_elem.items():
    if node not in nodes_domains.keys():
      nodes_domains[node] = set()
      for e in elems:
        if e in elem_to_subdomain.keys():
          nodes_domains[node].add(np.asscalar(elem_to_subdomain[e]))

  nodes_domains_sorted = {}
  for node, domains in nodes_domains.items():
    if len(domains) > 1:
      nodes_domains_sorted[node] = {}
      domains = sorted(domains)
      subdomain_cpus = {}
      for d in domains: # find all avaialble cpus
        cpus_id = set()
        for e in nodes_to_elem[node]:
          if elem_to_subdomain[e] == d:
            cpus_id.add(np.asscalar(elem_pid[e]))
        subdomain_cpus[d] = sorted(cpus_id)
      nodes_domains_sorted[node] = subdomain_cpus

  print(nodes_domains_sorted)
  return nodes_domains_sorted

def process_elem_node(data, serial=False):
  n = len(data)
  error = False
  for i in range(n):
    data_i=data[i]
    for j in range(n):

      if i != j or serial:
        # print("i!=j")
        data_j=data[j]
        for key, val in data_i.items():
          if key in data_j.keys():
            val2 = data_j[key]
            if not np.all(val == val2):
              error = True
              print("elem {} does not have the same nodes on cpu {} and {}".format(key, i, j))
              print("nodes on cpu {}: {}".format(i, val))
              print("nodes on cpu {}: {}".format(j, val2))

  if not error:
    print("Element Node Map is correct")


def flattenData(nodes_domains_cpu):
  flattend_node_domains = []
  for node, domains_cpu in nodes_domains_cpu.items():
    for d, cpu in domains_cpu.items():
      for c in cpu:
        flattend_node_domains.append(node)
        flattend_node_domains.append(d)
        flattend_node_domains.append(c)


  print(flattend_node_domains)
  return flattend_node_domains

def BreakMeshByBlockTest(nfiles):
  fname_base = "stdout.processor."
  for i in range(nfiles):
    fname = "{}{}".format(fname_base,i)
    elem_node_initial = read_data(fname, "OUT_ELEMENT_NODE_INITIAL_START","OUT_ELEMENT_NODE_INITIAL_END")
    print("element node map length on cpu {}: {} ".format(i, len(elem_node_initial)))
    elem_pid = read_data(fname, "OUT_ELEMENT_PID_START","OUT_ELEMENT_PID_END")
    print("elem pid map length on cpu {}: {} ".format(i, len(elem_pid)))
    element_subdomain = read_data(fname, "OUT_ELEMENT_SUBDOMAIN_START","OUT_ELEMENT_SUBDOMAIN_END")
    print("element subdomain map length on cpu {}: {} ".format(i, len(element_subdomain)))
    nodes_to_element = node_to_elem_map(elem_node_initial)
    print("node to element map length on cpu {}: {} ".format(i, len(nodes_to_element)))
    node_pid = read_data(fname, "OUT_NODE_PID_INITIAL_START","OUT_NODE_PID_INITIAL_END")
    print("node pid map length on cpu {}: {} ".format(i, len(node_pid)))
    nodes_domains_cpu = nodeDomainsCPU(nodes_to_element, element_subdomain, elem_pid)
    data = flattenData(nodes_domains_cpu)





    print("")
    print("")



if __name__=="__main__":
  n_files = 5
  BreakMeshByBlockTest(n_files)

  #compute_nnode(elem_node_initial)
  #nodes_domains_sorted, nodes_multiplicity,nnode_add = node_multiplicity(nodes_to_element,  element_sub)
  #print("number of nodes to add {} ".format(nnode_add))

  # elem_node_final = read_data(n_files, "OUT_ELEMENT_NODE_START","OUT_ELEMENT_NODE_END")
  # compute_nnode(elem_node_final)
  # node_pid = read_data(n_files, "OUT_NODE_PID_START","OUT_NODE_PID_END")
  # element_pid = read_data(n_files, "OUT_ELEMENT_PID_START","OUT_ELEMENT_PID_END")
  #
  #
  # # print(data)
  # process_elem_node(elem_node_final, serial=True)
