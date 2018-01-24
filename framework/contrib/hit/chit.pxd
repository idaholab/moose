
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool

cdef extern from "parse.h" namespace "hit":
    cdef cppclass NodeType:
        pass

cdef extern from "parse.h" namespace "hit::NodeType":
    cdef NodeType NTAll "hit::NodeType::All"
    cdef NodeType NTSection "hit::NodeType::Section"
    cdef NodeType NTComment "hit::NodeType::Comment"
    cdef NodeType NTField "hit::NodeType::Field"
    cdef NodeType NTBlank "hit::NodeType::Blank"

cdef extern from "parse.h" namespace "hit":
    cdef cppclass Node:
        Node(NodeType t)
        NodeType type()
        string path()
        string fullpath()
        int line()
        string render(int indent)

        string strVal() except +
        #bool boolVal()
        #int intVal()
        #double floatVal()
        #std::vector<double> vecFloatVal()
        #std::vector<int> vecIntVal()
        #std::vector<std::string> vecStrVal()

        void addChild(Node * child)
        vector[Node *] children(NodeType t)
        Node * parent()
        Node * root()
        Node * clone()

        #void walk(Walker * w, NodeType t = NodeType::Field);
        Node * find(const string & path)
        #template <typename T> T param(std::string path = "")

    Node * parse(string fname, string input) except +
    Node * explode(Node * n) except +
    void merge(Node * src, Node * dst) except +

cdef extern from "parse.h" namespace "hit::Field":
    cdef cppclass Kind:
        pass

cdef extern from "parse.h" namespace "hit::Field::Kind":
    cdef Kind None
    cdef Kind Int
    cdef Kind Float
    cdef Kind Bool
    cdef Kind String

cdef extern from "parse.h" namespace "hit":
    cdef cppclass Field "hit::Field":
        Field(const string & field, Kind k, const string & val)
        Kind kind()
        #vector[double] vecFloatVal()
        #vector[int] vecIntVal()
        #vector[string] vecStrVal()
        bool boolVal()
        int intVal()
        double floatVal()
        string strVal()

cdef extern from "parse.h" namespace "hit":
    cdef cppclass Comment "hit::Comment":
        Comment(const string & text, bool is_inline)

cdef extern from "parse.h" namespace "hit":
    cdef cppclass Blank "hit::Blank":
        Blank()

cdef extern from "parse.h" namespace "hit":
    cdef cppclass Section "hit::Section":
        Section(const string & path)

