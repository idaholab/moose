#!/usr/bin/env python
import os
import unittest
import MooseDocs

class TestMooseLinkDatabase(unittest.TestCase):

    @classmethod
    def setUpClass(cls):

        config = MooseDocs.load_config(os.path.join(MooseDocs.MOOSE_DIR, 'docs', 'moosedocs.yml'))
        options = config[0]['MooseDocs.extensions.MooseMarkdownExtension']
        cls.database = MooseDocs.MooseLinkDatabase(**options)

    def testTests(self):
        """
        Look for class in input files.
        """
        # The BoxMarker object is something that is nested and not usually listed first, so it is a good test case
        # that the regex is getting down into the nested items.
        self.assertIn('BoxMarker', self.database.inputs['Tests'], 'BoxMarker not located in database!')

    def testExamples(self):
        """
        The ExampleDiffusion class should be in input files and Kernel inherited from.
        """
        self.assertIn('ExampleDiffusion', self.database.inputs['Examples'], 'ExampleDiffusion was not found in example input files!')
        self.assertIn('Kernel', self.database.children['Examples'], 'Kernel was not used in the example source code!')

    def testTutorials(self):
        """
        Test the tutorials directory is properly searched.
        """
        self.assertIn('DarcyPressure', self.database.inputs['Tutorials'], 'DarcyPressue was not found in tutorial input files!')
        self.assertIn('Diffusion', self.database.children['Tutorials'], 'Diffusion was not used in the tutorial source code!')

    def testSource(self):
        """
        Test the tutorials directory is properly searched.
        """
        self.assertIn('Diffusion', self.database.children['Source'], 'Diffusion was not found in source code.!')


if __name__ == '__main__':
    unittest.main(verbosity=2)
