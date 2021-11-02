import gmshparser
import numpy as np
from scipy.optimize import fsolve
import itertools

def forcingFunction(a,diff,x,y):
    return 2*diff*np.sin(x)*np.cos(y)

def exactSolution(x,y):
    return np.sin(x)*np.cos(y)

def gaussGradient(element):
    print('somthing')

class FaceInfo:
    def __init__(self, mesh, face_connectivity, face_id, node_dict, boundary):

        self.node_dict = node_dict
        self.boundary = boundary
        self.face_id = face_id
        self.face_connectivity = face_connectivity
        self.dim = len(face_connectivity)
        self.face_tag = face_id

        self.face_center = None
        self.face_area = None
        self.face_nodes = None

        self.elem = None
        self.elem_center = None
        self.elem_nodes = None
        self.elem_volume = None

        self.neighbor = None
        self.neighbor_center = None
        self.neighbor_nodes = None
        self.neighbor_volume = None

        self.normal = None
        self.gc = None
        self.d_EN = None
        self.e_EN = None
        self.d_Cf = None
        self.d_Nf = None
        self.mag_d_EN = None
        self.m = None

    def computeCoefficients(self):

        if self.boundary:
            elems = getElements(mesh, self.face_connectivity)
            if len(elems) > 1:
                raise ValueError("Something is wrong with the mesh! (or the code)")
            self.elem = elems[0]
            self.neighbor = None
        else:
            elems = getElements(mesh, self.face_connectivity)
            if len(elems) != 2:
                raise ValueError("Something is wrong with the mesh! (or the code)")

            self.elem = elems[0]
            self.neighbor = elems[1]

        self.face_nodes = [np.asarray(self.node_dict[node])
                      for node in self.face_connectivity]

        self.face_center = np.mean(self.face_nodes, axis=0)

        self.face_area = np.linalg.norm(self.face_nodes[0]-self.face_nodes[1])

        self.elem_nodes = [np.asarray(self.node_dict[node])
                           for node in self.elem.get_connectivity()]

        self.elem_center = np.mean(self.elem_nodes, axis=0)

        self.elem_volume = np.abs(self.elem_nodes[0][0]*(self.elem_nodes[1][1]-self.elem_nodes[2][1]) \
                                  + self.elem_nodes[1][0]*(self.elem_nodes[2][1]-self.elem_nodes[0][1]) \
                                  + self.elem_nodes[2][0]*(self.elem_nodes[0][1]-self.elem_nodes[1][1])) / 2.0

        self.d_Cf = self.face_center - self.elem_center

        temp_vec= self.face_nodes[0]-self.face_nodes[1]
        self.normal = [-temp_vec[1], temp_vec[0], temp_vec[2]]/self.face_area
        if np.dot(self.normal,self.d_Cf) < 0.0:
            self.normal = -self.normal

        if self.boundary:

            self.d_EN = self.face_center - self.elem_center # self.normal * np.dot(self.normal, self.d_Cf)
            self.mag_d_EN = np.linalg.norm(self.d_EN)

            self.m = self.face_center - self.face_center
            # self.d_Cf - \
            #             np.dot(self.normal, self.d_Cf)/np.dot(self.normal, self.d_EN)*self.d_EN

        else:
            self.neighbor_nodes = [np.asarray(self.node_dict[node])
                               for node in self.neighbor.get_connectivity()]

            self.neighbor_center = np.mean(self.neighbor_nodes, axis=0)

            self.neighbor_volume = np.abs(self.neighbor_nodes[0][0]*(self.neighbor_nodes[1][1]-self.neighbor_nodes[2][1]) \
                                      + self.neighbor_nodes[1][0]*(self.neighbor_nodes[2][1]-self.neighbor_nodes[0][1]) \
                                      + self.neighbor_nodes[2][0]*(self.neighbor_nodes[0][1]-self.neighbor_nodes[1][1])) / 2.0

            self.d_Nf = self.face_center - self.neighbor_center

            self.d_EN = self.neighbor_center - self.elem_center

            self.mag_d_EN = np.linalg.norm(self.d_EN)

            self.m = self.d_Cf - \
                         np.dot(self.normal, self.d_Cf)/np.dot(self.normal, self.d_EN)*self.d_EN
            self.gc = np.linalg.norm(self.d_Nf - self.m)/self.mag_d_EN

        self.e_EN = self.d_EN / self.mag_d_EN

def getElements(mesh, face_connectivity):

    element_entities = mesh.get_element_entities()
    dim = len(face_connectivity)

    elems = []
    for elem_entity in element_entities:
        if elem_entity.get_dimension() == dim:
            for elem in elem_entity.get_elements():
                if set(face_connectivity).issubset(set(elem.get_connectivity())):
                    elems.append(elem)
    return elems

def computeCellCenters(mesh):

    node_entities = mesh.get_node_entities()

    node_dict = {}
    for node_entity in node_entities:
        for node in node_entity.get_nodes():
            node_dict[node.get_tag()] = node.get_coordinates()

    element_entities = mesh.get_element_entities()

    dim = 0
    for elem in element_entities:
        dim = max(dim, elem.get_dimension())

    cell_centers = {}
    for elem in element_entities:
        if elem.get_dimension() == dim:
            for el in elem.get_elements():

                elem_nodes = [np.asarray(node_dict[node])
                              for node in el.get_connectivity()]
                cell_centers[el.get_tag()]=np.mean(elem_nodes, axis=0)

    return cell_centers

def computeCellVolumes(mesh):

    node_entities = mesh.get_node_entities()

    node_dict = {}
    for node_entity in node_entities:
        for node in node_entity.get_nodes():
            node_dict[node.get_tag()] = node.get_coordinates()

    element_entities = mesh.get_element_entities()

    dim = 0
    for elem in element_entities:
        dim = max(dim, elem.get_dimension())

    cell_volumes = {}
    for elem in element_entities:
        if elem.get_dimension() == dim:
            for el in elem.get_elements():

                elem_nodes = [np.asarray(node_dict[node])
                              for node in el.get_connectivity()]
                cell_volumes[el.get_tag()]=np.abs(elem_nodes[0][0]*(elem_nodes[1][1]-elem_nodes[2][1]) \
                                          + elem_nodes[1][0]*(elem_nodes[2][1]-elem_nodes[0][1]) \
                                          + elem_nodes[2][0]*(elem_nodes[0][1]-elem_nodes[1][1])) / 2.0
    return cell_volumes

def computeFaceInfo(mesh):

    element_entities = mesh.get_element_entities()
    node_entities = mesh.get_node_entities()
    face_info = {}

    dim = 0
    for elem in element_entities:
        dim = max(dim, elem.get_dimension())

    if (dim > 2):
        raise ValueError("Only 2D meshes are supported!")

    node_dict = {}
    for node_entity in node_entities:
        for node in node_entity.get_nodes():
            node_dict[node.get_tag()] = node.get_coordinates()

    faces = []
    for elem in element_entities:
        if elem.get_dimension() == dim - 1:
            for face in elem.get_elements():
                faces.append((True, face.get_connectivity()))

        else:
            for el in elem.get_elements():
                faces_tmp = itertools.combinations(el.get_connectivity(), dim)
                faces += [(False, list(i)) for i in faces_tmp]

    unique_faces = []
    for face in faces:
        matching = False
        for u_face in unique_faces:
            if set(face[1]).issubset(set(u_face[1])):
                matching = True
                break

        if matching:
            continue
        else:
            unique_faces.append(face)

    face_info = []
    for face_id in range(len(unique_faces)):
        face_info.append(FaceInfo(mesh, unique_faces[face_id][1],
                                  face_id, node_dict, unique_faces[face_id][0]))
        face_info[face_id].computeCoefficients()

    return face_info

def computeElemConnectivity(mesh, face_info):

    element_entities = mesh.get_element_entities()

    dim = 0
    for elem in element_entities:
        dim = max(dim, elem.get_dimension())

    elem_to_face = {}
    shift = 0
    for elem in element_entities:
        if elem.get_dimension() == dim:
            for el in elem.get_elements():
                shift = el.get_tag() if shift == 0 else shift
                e2f = []
                for face in face_info:
                    if set(face.face_connectivity).issubset(set(el.get_connectivity())):
                        e2f.append(face.face_id)
                elem_to_face[el.get_tag()]=e2f
    return shift, elem_to_face,

def computeFaceValue(face_info, face_id, elem_id, shift, solution, iters, max_iter, ignore_skewness=False):

    fi = face_info[face_id]

    iters = iters + 1

    if fi.boundary:
        fc = fi.face_center
        print("        BFV ", fi.face_center, exactSolution(fc[0],fc[1]))
        return exactSolution(fc[0],fc[1])

    nb_id = fi.neighbor.get_tag()

    e_id = fi.elem.get_tag() if fi.elem.get_tag() == elem_id else fi.neighbor.get_tag()
    n_id = fi.neighbor.get_tag() if fi.elem.get_tag() == elem_id else fi.elem.get_tag()

    gc = fi.gc if fi.elem.get_tag() == elem_id else (1. - fi.gc)

    v_el = solution[e_id - shift]
    v_nb = solution[n_id - shift]

    face_value = gc*v_el + (1-gc)*v_nb

    if iters <= max_iter:
        fg = computeFaceGradient(face_info, face_id, elem_id, elem_to_face,
                                 shift, solution, iters, max_iter)
        face_value += np.dot(fg,fi.m)

    print("        IFV ", fi.face_center, face_value)
    return face_value

def computeCellGradient(face_info, elem_to_face, elem_id,
                        shift, solution, iters, max_iters, ignore_skewness=False):

    faces = elem_to_face[elem_id]

    print("    ELEM: ", elem_id)

    cell_gradient = np.zeros(3)
    for face_id in faces:
        fi = face_info[face_id]
        face_normal = fi.normal if elem_id == fi.elem.get_tag() else -fi.normal
        face_area = fi.face_area
        face_value = computeFaceValue(face_info, face_id, elem_id, shift, solution, iters, max_iters)
        cell_gradient += face_value * face_normal * face_area

    cell_volume = fi.elem_volume if elem_id == fi.elem.get_tag() else fi.neighbor_volume
    cell_gradient /= cell_volume

    print("    gradient: ", cell_gradient)

    return cell_gradient

def computeFaceGradient(face_info, face_id, elem_id, elem_to_face,
                        shift, solution, iters, max_iters, ignore_skewness=False):


    fi = face_info[face_id]

    print("    Computing face grad for: ", fi.face_center)
    face_normal = fi.normal if elem_id == fi.elem.get_tag() else -fi.normal

    e_EN = fi.e_EN if elem_id == fi.elem.get_tag() else -fi.e_EN

    face_grad = np.zeros(3)
    if fi.boundary:
        v_el = solution[elem_id - shift]
        v_bd = computeFaceValue(face_info, face_id, elem_id, shift,
                                solution, iters, max_iters)

        approx_grad = computeCellGradient(face_info, elem_to_face, elem_id,
                                          shift, solution, iters, max_iters)

        print("      Uncorr FG ",fi.face_center," ", approx_grad)

        approx_grad -= np.dot(approx_grad,e_EN)*e_EN

        face_grad = (v_bd - v_el)/fi.mag_d_EN * e_EN + approx_grad

        print("      Corr FG ",fi.face_center," ", face_grad)


    else:

        e_id = fi.elem.get_tag() if elem_id == fi.elem.get_tag() else fi.neighbor.get_tag()
        n_id = fi.neighbor.get_tag() if elem_id == fi.elem.get_tag() else fi.elem.get_tag()
        gc = fi.gc if fi.elem.get_tag() == elem_id else (1. - fi.gc)

        e_grad = computeCellGradient(face_info, elem_to_face, e_id, shift, solution, iters, max_iters)
        n_grad = computeCellGradient(face_info, elem_to_face, n_id, shift, solution, iters, max_iters)

        if iters == max_iters -1:
            print("    ELEM1 G: ", e_grad)
            print("    Elem2 G: ", n_grad)

        approx_grad = gc * e_grad + (1 - gc) * n_grad

        print("      Uncorr FG",fi.face_center," ", approx_grad)

        approx_grad -= np.dot(approx_grad,e_EN)*e_EN

        v_el = solution[e_id - shift]
        v_nb = solution[n_id - shift]
        face_grad = ((v_nb - v_el) / fi.mag_d_EN) * e_EN + approx_grad

        print("      Corr FG",fi.face_center," ", face_grad)

    return face_grad

def BuildResidual(solution, diff, a, face_info, elem_to_face, shift, cell_centers, cell_volumes, iter, max_iter):

    res = np.zeros(len(solution))
    for elem_id in elem_to_face:
        print("E: ", elem_id, cell_centers[elem_id])
        for face_id in elem_to_face[elem_id]:
            fi = face_info[face_id]
            face_normal = fi.normal if elem_id == fi.elem.get_tag() else -fi.normal
            print('   F BEGIN: ', fi.face_center)
            fg = computeFaceGradient(face_info, face_id, elem_id, elem_to_face, shift, solution, iter, max_iter)

            print('   F END: ', fi.face_center, " FG*Sn: ",np.dot(fg,face_normal), ' k: ', diff)
            res[elem_id - shift] -= np.dot(fg,face_normal)*diff*fi.face_area
        cell_volume = cell_volumes[elem_id]
        cell_center = cell_centers[elem_id]
        res[elem_id - shift] -= forcingFunction(a,diff,cell_center[0],cell_center[1])*cell_volume
    return res

skewness_correction = False
non_orthogonal_correction = False

def computeL2Error(cell_centers, cell_volumes, solution, shift):
    diff = 0.0
    for elem_id in cell_centers:
        diff += ((solution[elem_id-shift]-exactSolution(cell_centers[elem_id][0],cell_centers[elem_id][1]))**2)*cell_volumes[elem_id]
    diff = np.sqrt(diff)

    return diff

a = 1.1
diff=1.1

refinements=0
for ref_lvl in range(refinements+1):
    mesh_name = 'skewed-'+str(ref_lvl)+'.msh'

    mesh = gmshparser.parse(mesh_name)
    cell_centers = computeCellCenters(mesh)
    cell_volumes = computeCellVolumes(mesh)

    face_info = computeFaceInfo(mesh)

    shift, elem_to_face = computeElemConnectivity(mesh, face_info)

    solution = np.ones(mesh.get_max_element_tag()-shift+1)

    iter = 0
    max_iter = 1
    # res = BuildResidual(solution, diff, a, face_info, elem_to_face, shift, cell_centers, cell_volumes)
    solution = fsolve(BuildResidual, solution, args=(diff, a, face_info, elem_to_face,
                      shift, cell_centers, cell_volumes, iter, max_iter))

    error = computeL2Error(cell_centers, cell_volumes, solution, shift)

    print('Lvl: ',ref_lvl,' Error: ',error)
