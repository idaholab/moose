#!/usr/bin/env python3
import sys
import unittest
import chigger

class TestChiggerAlgorithm(unittest.TestCase):
    def testUpdate(self):
        obj = chigger.base.ChiggerAlgorithm()
        with self.assertLogs(level='DEBUG') as log:
            obj.update()

        self.assertEqual(len(log.output), 5)
        self.assertIn('update', log.output[0])
        self.assertIn('RequestInformation', log.output[1])
        self.assertIn('_onRequestInformation', log.output[2])
        self.assertIn('RequestData', log.output[3])
        self.assertIn('_onRequestData', log.output[4])

        with self.assertLogs(level='DEBUG') as log:
            obj.update()
        self.assertEqual(len(log.output), 1)
        self.assertIn('update', log.output[0])

    def testUpdateInformation(self):
        obj = chigger.base.ChiggerAlgorithm()
        with self.assertLogs(level='DEBUG') as log:
            obj.updateInformation()

        self.assertEqual(len(log.output), 3)
        self.assertIn('updateInformation', log.output[0])
        self.assertIn('RequestInformation', log.output[1])
        self.assertIn('_onRequestInformation', log.output[2])

        with self.assertLogs(level='DEBUG') as log:
            obj.updateInformation()
        self.assertEqual(len(log.output), 1)
        self.assertIn('updateInformation', log.output[0])

    def testUpdateData(self):
        obj = chigger.base.ChiggerAlgorithm()
        with self.assertLogs(level='DEBUG') as log:
            obj.updateData()

        self.assertEqual(len(log.output), 5)
        self.assertIn('updateData', log.output[0])
        self.assertIn('RequestInformation', log.output[1])
        self.assertIn('_onRequestInformation', log.output[2])
        self.assertIn('RequestData', log.output[3])
        self.assertIn('_onRequestData', log.output[4])

        with self.assertLogs(level='DEBUG') as log:
            obj.updateData()
        self.assertEqual(len(log.output), 1)
        self.assertIn('updateData', log.output[0])

    def testSetOption(self):
        obj = chigger.base.ChiggerAlgorithm()
        t0 = obj.GetMTime()

        obj.setParam('name', 'andrew')
        t1 = obj.GetMTime()
        self.assertTrue(t1 > t0)

        obj.setParam('name', 'andrew')
        t2 = obj.GetMTime()
        self.assertEqual(t1, t2)

    def testSetParams(self):
        obj = chigger.base.ChiggerAlgorithm()
        t0 = obj.GetMTime()

        obj.setParams(name='andrew')
        t1 = obj.GetMTime()
        self.assertTrue(t1 > t0)

        obj.setParams(name='andrew')
        t2 = obj.GetMTime()
        self.assertEqual(t1, t2)

if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
