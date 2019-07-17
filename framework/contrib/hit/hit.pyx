
cimport chit

from libcpp.string cimport string
from libcpp cimport bool as cbool
from libcpp.vector cimport vector

class NodeType(object):
    All = 'All'
    Section = 'Section'
    Comment = 'Comment'
    Field = 'Field'
    Blank = 'Blank'

cdef chit.NodeType _nodetype_enum(node_type):
    if node_type == NodeType.All:
        return chit.NTAll
    elif node_type == NodeType.Section:
        return chit.NTSection
    elif node_type == NodeType.Comment:
        return chit.NTComment
    elif node_type == NodeType.Field:
        return chit.NTField
    elif node_type == NodeType.Blank:
        return chit.NTBlank
    raise RuntimeError('unknown NodeType ' + node_type)

class FieldKind(object):
    Int = 'Int'
    Float = 'Float'
    Bool = 'Bool'
    String = 'String'
    NotField = 'None'

cdef chit.Kind _kind_enum(kind):
    if kind == FieldKind.Int:
        return chit.Int
    elif kind == FieldKind.Float:
        return chit.Float
    elif kind == FieldKind.Bool:
        return chit.Bool
    elif kind == FieldKind.String:
        return chit.String
    raise RuntimeError('unknown Field::Kind ' + kind)

cpdef NewField(name, kind, val):
    cppname = <string> name.encode('utf-8')
    cppval = <string> val.encode('utf-8')
    cdef chit.Node* f = <chit.Node*> new chit.Field(cppname, _kind_enum(kind), cppval)
    return _initpynode(f)

cpdef NewSection(path):
    cpath = <string> path.encode('utf-8')
    cdef chit.Node* f = <chit.Node*> new chit.Section(cpath)
    return _initpynode(f)

cpdef NewComment(text, is_inline=False):
    ctext = <string> text.encode('utf-8')
    cdef chit.Node* f = <chit.Node*> new chit.Comment(ctext, <cbool>is_inline)
    return _initpynode(f)

cpdef NewBlank():
    cdef chit.Node* f = <chit.Node*> new chit.Blank()
    return _initpynode(f)

cdef class Formatter:
    cdef chit.Formatter _formatter

    def __cinit__(self, style_file=''):
        self._formatter = chit.Formatter()
        if style_file != '':
            with open(style_file, 'r') as f:
                data = f.read()
            self._formatter = chit.Formatter(style_file, data)

    def addPattern(self, prefix, order):
        cdef vector[string] order_vec
        for o in order:
            order_vec.push_back(o)
        self._formatter.addPattern(prefix, order_vec)

    def format(self, fname, content):
        return str(self._formatter.format(fname, content))

cdef class Node:
    cdef chit.Node* _cnode
    cdef cbool _own
    cdef str fname

    @classmethod
    def NewSection(cls, path):
        pass

    @classmethod
    def NewComment(cls, text):
        pass

    @classmethod
    def NewBlank(cls):
        pass

    def __cinit__(self, own=False, fname=''):
        self._cnode = NULL
        self._own = own
        self.fname = fname
        pass

    def __dealloc__(self):
        if self._cnode != NULL and self._own:
            del self._cnode

    def __deepcopy__(self, memodict):
        return self.clone()

    def __reduce__(self):
        return (parse, (self.fname, self.render()))

    def __repr__(self):
        return self.render()

    def render(self, indent=0, indent_text='  ', maxlen=0):
        cindent = <string> indent_text.encode('utf-8')
        return self._cnode.render(indent, cindent, maxlen)

    def line(self):
        return int(self._cnode.line())

    def path(self):
        return str(self._cnode.path())
    def fullpath(self):
        return str(self._cnode.fullpath())
    def type(self):
        t = <int>self._cnode.type()
        if t == <int>chit.NTField:
            return NodeType.Field
        elif t == <int>chit.NTSection:
            return NodeType.Section
        elif t == <int>chit.NTComment:
            return NodeType.Comment
        elif t == <int>chit.NTBlank:
            return NodeType.Blank
        else:
            return 'Unknown'

    def kind(self):
        if self.type() != NodeType.Field:
            return FieldKind.NotField

        f = <chit.Field *> self._cnode
        k = <int>f.kind()
        if k == <int>chit.Int:
            return FieldKind.Int
        elif k == <int>chit.Float:
            return FieldKind.Float
        elif k == <int>chit.Bool:
            return FieldKind.Bool
        elif k == <int>chit.String:
            return FieldKind.String
        return FieldKind.NotField

    def raw(self):
        if self.type() != NodeType.Field:
            return None
        return str(self._cnode.strVal())

    def find(self, path):
        cpath = <string> path.encode('utf-8')
        n = self._cnode.find(cpath)
        if n == NULL:
            return None
        return _initpynode(n)

    def param(self, path=''):
        cpath = <string> path.encode('utf-8')
        n = self._cnode.find(cpath)
        if path != '' and n == NULL:
            return None
        elif path == '':
            n = self._cnode

        cdef Node nn = _initpynode(n)
        if nn.type() != NodeType.Field:
            return None

        f = <chit.Field *> nn._cnode
        k = nn.kind()
        if k == FieldKind.Int:
            return int(f.intVal())
        elif k == FieldKind.Float:
            return float(f.floatVal())
        elif k == FieldKind.Bool:
            return bool(f.boolVal())
        return str(f.strVal())

    def walk(self, walker, node_type=NodeType.All):
        if self.type() == node_type or node_type == NodeType.All:
            walker.walk(self.fullpath(), self.path(), self);
        for child in self.children():
            child.walk(walker, node_type);

    def clone(self):
        return _initpynode(self._cnode.clone(), own=self._own)
    def root(self):
        return _initpynode(self._cnode.root())
    def parent(self):
        return _initpynode(self._cnode.root())
    def addChild(self, Node child):
        self._cnode.addChild(child._cnode)
    def children(self, node_type = NodeType.All):
        ckids = self._cnode.children(_nodetype_enum(node_type));
        kids = []
        for val in ckids:
            kids.append(_initpynode(val))
        return kids

# this function is a hack to get around the fact that cython assumes all arguments to class
# constructors are python objects.  So the Node constructor does nothing and this function
# actually sets the internal cnode member pointer.
cdef _initpynode(chit.Node* n, own=False):
    pyn = Node(own=own)
    pyn._cnode = n
    return pyn

def parse(fname, input):
    cdef chit.Node* node = chit.parse(fname.encode('utf-8'), input.encode('utf-8'))
    return _initpynode(node, own=True)

cpdef explode(Node n):
    n._cnode = chit.explode(n._cnode)
    return n

cpdef merge(Node src, Node dst):
    chit.merge(src._cnode, dst._cnode)
