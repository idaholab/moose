import unittest
import tempfile
import os

import hit

class TestHIT(unittest.TestCase):
    def test_install(self):
        loc = str(hit.__file__)
        self.assertTrue(loc)
        self.assertTrue(loc.endswith(".so") or loc.endswith(".dylib"))

    def test_FailCases(self):
        cases = [
            ("comment in path", "[hello#world] []"),
            ("comment in field", "hello#world=foo"),
            ("missing string", "[hello] foo = []"),
            ("missing string 2", "[hello] foo = \n bar = 42[]"),
            ("invalid path char '='", "[hello=world] []"),
            ("invalid path char '&'", "[hello&world] []"),
            ("invalid path char '['", "[hello[world] []"),
            ("invalid field char '&'", "hello&world=foo"),
            ("invalid field char '['", "hello[world=foo"),
            ("unfinished field", "hello\nfoo=bar"),
            ("unterminated section", "[hello]"),
            ("unterminated section", "[hello][./]"),
            ("extra section close", "[]"),
            ("extra section close 2", "[../]"),
            ("empty  dotslash section name", "[./][]"),
            ("mismatched consecutive string literal quotes", "foo='bar'\"baz\""),
        ]

        for name, input in cases:
            with self.assertRaises(RuntimeError, msg=name):
                hit.parse("", input)

    def test_LineNumbers(self):
        cases = [
            ("[hello] foo='bar'\n\n\n boo='far'\n\n[]", [1, 1, 4]),
            ("[hello]\n  foo='bar'\n[]\n[goodbye]\n  boo=42\n[]", [1, 2, 4, 5]),
            ("[hello/bar]\n  foo=42\n[]", [1, 1, 2]),
            ("[hello]\n\n # comment\n foo='bar' 'baz' # another comment\n\nboo=42[]", [1, 3, 4, 4, 6]),
        ]

        class LineWalker:
            def __init__(self, want_lines):
                self.want = want_lines
                self.count = 0

            def walk(self, fullpath, _nodepath, node):
                if (node.type() == hit.NodeType.Blank or not fullpath):
                    return
                assert self.count < len(self.want)
                assert self.want[self.count] == node.line()
                self.count += 1

        for input, line_nums in cases:
            walker = LineWalker(line_nums)
            node = hit.parse("", input)
            node.walk(walker)

    def test_PassCases(self):
        cases = [
            ("valid special field chars", "hello_./:<>-+world=foo"),
            ("comment in field", "hello=wo#rld"),
            ("bad number becomes string", "foo=4.2abc"),
            ("empty section close", "[hello] []"),
            ("dotdotslash section close", "[hello] [../]"),
            ("no whitespace between headers/footers", "[hello][../]"),
            ("no whitespace between headers/footers", "[hello][]"),
            ("no whitespace with sections and fields", "[hello][world]foo=bar[]baz=42[]"),
            ("no leading ./ in sub-block", "[hello] [world] [] []"),
            ("consecutive string literals", "foo='bar''baz'"),
            ("no infinite loop", "foo='bar'\n\n "),
        ]
        for name, input in cases:
            try:
                hit.parse("", input)
            except:
                self.fail(name)

    def test_ExplodeParentless(self):
        node = hit.NewSection("foo/bar")
        content = node.render()
        self.assertEqual(content, "[foo]\n  [bar]\n  []\n[]")

    def test_RenderParentlessSection(self):
        node = hit.NewSection("mypath")
        content = node.render()
        self.assertEqual(content, "[mypath]\n[]")

    def test_RenderSubsection(self):
        root = hit.parse("", "[hello][world]foo=42[][]")
        node = root.find("hello/world")
        content = node.render()
        self.assertEqual(content, "[world]\n  foo = 42\n[]")

    def test_RenderCases(self):
        test_cases = [
            (
                "root level fields",
                "foo=bar boo=far",
                "foo = bar\nboo = far",
                0,
            ),
            (
                "single section",
                "[foo]bar=baz[../]",
                "[foo]\n  bar = baz\n[../]",
                0,
            ),
            (
                "remove leading newline",
                "\n[foo]bar=baz[../]",
                "[foo]\n  bar = baz\n[../]",
                0,
            ),
            (
                "preserve consecutive newline",
                "[foo]\n\nbar=baz[../]",
                "[foo]\n\n  bar = baz\n[../]",
                0,
            ),
            (
                "reflow long string",
                "foo=\"hello my name is joe and I work in a button factory\"",
                "foo = \"hello my name is joe \"\n      \"and I work in a \"\n      \"button factory\"",
                28,
            ),
            (
                "don't reflow single quoted long string",
                "foo='hello my name is joe and I work in a button factory'",
                "foo = 'hello my name is joe and I work in a button factory'",
                28,
            ),
            (
                "don't reflow unquoted string",
                "foo=unquotedstring",
                "foo = unquotedstring",
                5,
            ),
            (
                "don't reflow single quoted string",
                "foo='longstring'",
                "foo = 'longstring'",
                12,
            ),
            (
                "reflow double quoted string",
                "foo=\"longstring\"",
                "foo = \"longst\"\n      \"ring\"",
                12,
            ),
            (
                "reflow pre-broken strings",
                "foo='why'\n' separate '  'strings?'",
                "foo = 'why separate strings?'",
                0,
            ),
            (
                "preserve quotes preceding blankline",
                "foo = '42'\n\n",
                "foo = '42'",
                0,
            ),
            (
                "preserve block comment (#10889)",
                "[hello]\n  foo = '42'\n\n  # comment\n  bar = 'baz'\n[]",
                "[hello]\n  foo = '42'\n\n  # comment\n  bar = 'baz'\n[]",
                0,
            ),
            (
                "preserve block comment 2 (#10889)",
                "[hello]\n  foo = '42'\n  # comment\n  bar = 'baz'\n[]",
                "[hello]\n  foo = '42'\n  # comment\n  bar = 'baz'\n[]",
                0,
            ),
            (
                "complex newline render",
                "[section01]\n\n  field01 = 10\n\n\n\n  field02 = '20'\n\n  [section02]"
                "\n\n    field03 = '30 31 32 33'\n\n\n    field04 = 40\n    [section03]"
                "\n\n\n\n\n\n      field05 = \"double 50 quoted 51 string\"\n\n\n    []"
                "\n\n\n    field06 = 60\n\n\n\n  []\n  field07 = '70 71 72 73 74'\n\n[]",
                "[section01]\n\n  field01 = 10\n\n  field02 = '20'\n\n  [section02]\n\n  "
                "  field03 = '30 31 32 33'\n\n    field04 = 40\n    [section03]\n\n      "
                "field05 = \"double 50 quoted 51 string\"\n    []\n\n    field06 = 60\n  "
                "[]\n  field07 = '70 71 72 73 74'\n[]",
                0,
            ),
        ]

        for name, input, output, maxlen in test_cases:
            root = hit.parse("", input)
            content = root.render(maxlen=maxlen)
            self.assertEqual(content, output, name)

    def test_MergeTree(self):
        root1 = hit.parse("", "[foo]bar=42[]")
        root2 = hit.parse("", "foo/baz/boo=42")
        hit.merge(root2, root1)
        self.assertEqual(root1.render(), "[foo]\n  bar = 42\n  [baz]\n    boo = 42\n  []\n[]")

        root1 = hit.parse("", "foo/bar=baz")
        root2 = hit.parse("", "foo/bar=42")
        hit.merge(root2, root1)
        node = root1.find("foo/bar")
        self.assertEqual(node.type(), hit.NodeType.Field)
        self.assertEqual(node.kind(), hit.FieldKind.Int)

    def test_Clone(self):
        root1 = hit.parse("", "[foo][bar]baz=42[][]")
        root2 = hit.NewSection("")
        root2.addChild(root1.children()[0].children()[0].children()[0].clone())
        self.assertEqual("baz = 42", root2.render())


class TestHITWalkers(unittest.TestCase):
    @unittest.skip("Python binding does not have a BraceExpander class.")
    def test_ParseFields(self):
        pass

    @unittest.skip("Python binding does not have a GatherParamWalker class.")
    def test_GatherParamWalker(self):
        pass

    @unittest.skip("Python binding does not have a RemoveParamWalker class.")
    def test_RemoveParamWalker(self):
        pass

    @unittest.skip("Python binding does not have a RemoveEmptySectionWalker class.")
    def test_RemoveEmptySectionWalker(self):
        pass

class TestHITFormat(unittest.TestCase):
    # @unittest.skip("Python binding for formatter is not working.")
    def test_Formatter(self):
        cases = [
            (
                "[format]line_length=100[]",
                "foo='line longer than 20 characters'",
                "foo = 'line longer than 20 characters'",
            ),
            (
                "[format]line_length=20[]",
                "foo=\"line longer than 20 characters\"",
                "foo = \"line longer \"\n      \"than 20 \"\n      \"characters\"",
            ),
            (
                "[format]canonical_section_markers=true[]",
                "[./foo][../]",
                "[foo]\n[]",
            ),
            (
                "[format]canonical_section_markers=false[]",
                "[./foo][../]",
                "[./foo]\n[../]",
            ),
            (
                "[format]indent_string='    '[]",
                "[foo]bar=42[]",
                "[foo]\n    bar = 42\n[]",
            ),
            (
                "[format]indent_string='      '[]",
                "[foo]bar=42[]",
                "[foo]\n      bar = 42\n[]",
            ),
        ]

        with tempfile.TemporaryDirectory() as temp_dir:
            for fmt, input, output in cases:
                temp_file = os.path.join(temp_dir, "style")
                with open(temp_file, "w") as tid:
                    tid.write(fmt)
                root = hit.parse("", input)
                fmter = hit.Formatter(temp_file)
                content = fmter.format("", input)
                self.assertEqual(content, output)

    def test_Formatter_sorting(self):
        cases = [
            (
                "order = bar outof = foo",
                "outof = foo\norder = bar",
                [
                    ("", ["outof", "order"]),
                    ("pattern2", ["param1", "param2"])
                ],
            ),
            (
                "order = bar outof = foo",
                "order = bar\noutof = foo",
                [],
            ),
        ]

        for input, output, patterns in cases:
            fmter = hit.Formatter()
            for pattern, order in patterns:
                fmter.addPattern(pattern, order)

            root = hit.parse("", input)
            fmter.formatTree(root)
            self.assertEqual(root.render(), output)

if __name__ == "__main__":
    unittest.main()
