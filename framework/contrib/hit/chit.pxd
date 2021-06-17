
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
        const string & filename()
        string render(int indent, const string & indent_text, int maxlen)

        string strVal() except +
        #bool boolVal()
        #int intVal()
        #double floatVal()
        #std::vector<double> vecFloatVal()
        #std::vector<int> vecIntVal()
        #std::vector<std::string> vecStrVal()

        void addChild(Node * child)
        void insertChild(int, Node * child)
        vector[Node *] children(NodeType t)
        Node * parent()
        Node * root()
        Node * clone()
        void remove()

        #void walk(Walker * w, NodeType t = NodeType::Field);
        Node * find(const string & path)
        #template <typename T> T param(std::string path = "")

    Node * parse(string fname, string input) except +
    Node * explode(Node * n) except +
    void merge(Node * src, Node * dst) except +

cdef extern from "parse.h" namespace "hit":
    cdef cppclass Formatter:
        Formatter()
        Formatter(const string & fname, const string & hit_config)

        string format(const string & fname, const string & input)
        void format(Node *)

        void addPattern(const string & prefix, const vector[string] & order)

        bool canonical_section_markers
        int line_length
        string indent_string

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
        bool boolVal()
        int intVal()
        double floatVal()
        string strVal()
        void setVal(const string & val, Kind k)

cdef extern from "parse.h" namespace "hit":
    cdef cppclass Comment "hit::Comment":
        Comment(const string & text, bool is_inline)
        void setText(const string & text)

cdef extern from "parse.h" namespace "hit":
    cdef cppclass Blank "hit::Blank":
        Blank()

cdef extern from "parse.h" namespace "hit":
    cdef cppclass Section "hit::Section":
        Section(const string & path)

# LEXER
cdef extern from "lex.h" namespace "hit":
    cdef cppclass TokType:
        pass

cdef extern from "lex.h" namespace "hit::TokType":
    cdef TokType TokenError "hit::TokType::Error"
    cdef TokType TokenEOF "hit::TokType::EOF"
    cdef TokType TokenEquals "hit::TokType::Equals"
    cdef TokType TokenLeftBracket "hit::TokType::LeftBracket"
    cdef TokType TokenRightBracket "hit::TokType::RightBracket"
    cdef TokType TokenIdent "hit::TokType::Ident"
    cdef TokType TokenPath "hit::TokType::Path"
    cdef TokType TokenNumber "hit::TokType::Number"
    cdef TokType TokenString "hit::TokType::String"
    cdef TokType TokenComment "hit::TokType::Comment"
    cdef TokType TokenInlineComment "hit::TokType::InlineComment"
    cdef TokType TokenBlankLine "hit::TokType::BlankLine"

cdef extern from "lex.h" namespace "hit":
    cdef cppclass Token:
        Token(TokType t, const string & val, const string & name, size_t offset, line)
        string str()
        TokType type
        string val
        string name
        size_t offset
        int line

cdef extern from "lex.h" namespace "hit":
    vector[Token] tokenize(string fname, string input) except +
