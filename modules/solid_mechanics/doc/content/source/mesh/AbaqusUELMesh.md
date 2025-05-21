# AbaqusUELMesh

!syntax description /Mesh/AbaqusUELMesh

The `AbaqusUELMesh` builds a mesh from an Abaqus input file with custom UEL (user elements). UELs do not have a corresponding representation in Libmesh and represented using Node-Elements and a custom connectivity map.

# Abaqus Input File Parser Design Documentation

## High-Level Overview

The Abaqus Input File Parser is a component of the MOOSE framework, designed to read and interpret Abaqus input files. These files contain complex finite element model definitions, which include nodes, elements, material properties, boundary conditions, and more. This parser converts the Abaqus input data into a format compatible with MOOSE's finite element framework, allowing for the integration of custom user elements and mesh management.

The parser is composed of several interconnected components, each responsible for handling specific parts of the Abaqus input files. These components work together to parse the input data, build the finite element model, and manage the resulting mesh.

## Components

### 1. `AbaqusUELMesh`
- **Description**: Inherits from `MooseMesh` and serves as the main entry point for handling Abaqus input files. It manages the mesh creation, node and element instantiation, and subdomain setup.
- **Key Functions**:
  - `buildMesh()`: Opens and parses the Abaqus input file, builds the model data structures, and sets up the mesh.
  - `instantiateElements()`: Adds mesh points and elements based on the parsed model data.
  - `setupLibmeshSubdomains()`: Configures subdomains for variable restrictions.
  - `setupNodeSets()`: Sets up node sets based on Abaqus input definitions.

### 2. `Abaqus::InputParser`
- **Description**: Parses the Abaqus input file and constructs a syntax tree representing the hierarchical structure of the input file.
- **Key Functions**:
  - `parse()`: Main function to load and parse the entire input file.
  - `parseBlock()`: Recursively parses blocks of the input file.
  - `parseOption()`: Parses individual options within a block.

### 3. `Abaqus::Model`
- **Description**: Represents the overall finite element model, combining parts, steps, and initial conditions.
- **Key Data Structures**:
  - `_nodes`: Stores all nodes in the model.
  - `_elements`: Stores all elements in the model.
  - `_nsets`, `_elsets`: Node sets and element sets respectively.
  - `_field_ics`: Field initial conditions.
  - `_step`: Steps containing boundary conditions and other step-specific data.

### 4. `Abaqus::Part`
- **Description**: Represents a part of the model, including nodes, elements, and their definitions.
- **Key Functions**:
  - `parse()`: Parses the part block in the input file.
  - `processNodeSet()`, `processElementSet()`: Processes node and element sets within the part.

### 5. `Abaqus::Step`
- **Description**: Represents a simulation step, including boundary conditions and time stepping information.
- **Key Functions**:
  - `parse()`: Parses the step block in the input file.
  - `optionFunc()`: Processes different options within a step, like boundary conditions.

### 6. `Abaqus::Assembly`
- **Description**: Represents an assembly of parts and instances within the model.
- **Key Functions**:
  - `parse()`: Parses the assembly block in the input file.

### 7. `Abaqus::ObjectStore<T>`
- **Description**: Generic class to store and manage objects of type `T` by name and ID. This class works like a vector in which elements can be accessed by an index or by a string name.
- **Key Functions**:
  - `add()`: Adds a new object to the store.
  - `merge()`: Merges another `ObjectStore` into the current one.

## Integration and Workflow

1. **Initialization**: An `AbaqusUELMesh` object is created with the necessary parameters, including the path to the Abaqus input file.

2. **Parsing**: The `buildMesh()` function of `AbaqusUELMesh` opens and parses the input file using `Abaqus::InputParser`. The parser constructs a syntax tree representing the hierarchical structure of the input file.

3. **Model Construction**:
   - Depending on the input file format, either a `FlatModel` or `AssemblyModel` is created.
   - The selected model class (`FlatModel` or `AssemblyModel`) parses the syntax tree to build the model data structures, including nodes, elements, and sets.

4. **Mesh Instantiation**: The `instantiateElements()` function of `AbaqusUELMesh` adds mesh points and elements to the MOOSE mesh based on the parsed model data.

5. **Subdomain and Node Set Setup**: The `setupLibmeshSubdomains()` and `setupNodeSets()` functions configure subdomains and node sets within the mesh.

6. **Finalization**: The mesh is prepared for use, and any additional debug or setup steps are performed.

This design provides a robust and flexible framework for integrating Abaqus input files with MOOSE, enabling advanced finite element simulations with custom user elements.

## Example Input File

!listing modules/solid_mechanics/test/tests/uel_mesh/abaqus_uel_read.i block=Mesh

!syntax parameters /Mesh/AbaqusUELMesh

!syntax inputs /Mesh/AbaqusUELMesh

!syntax children /Mesh/AbaqusUELMesh

!bibtex bibliography
