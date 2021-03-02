#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import collections
import unittest
import mooseutils.civet_results as cr

SITE = 'https://civet.inl.gov'
REPO = 'idaholab/moose'
SHAS = ['681ba2f4274dc8465bb2a54e1353cfa24765a5c1', 'febe3476040fe6af1df1d67e8cc8c04c4760afb6']


class Test(unittest.TestCase):
    def testGetCivetJobs(self):
        jobs = cr._get_remote_civet_jobs(SHAS, SITE, REPO)
        self.assertEqual(len(jobs), 67)
        self.assertEqual(jobs[0].number, 443457)
        self.assertTrue(jobs[0].filename.endswith('results_443457.tar.gz'))
        self.assertEqual(jobs[0].url, SITE)

    def testUpdateDatabaseFromJob(self):
        jobs = cr._get_remote_civet_jobs(SHAS, SITE, REPO)
        database = collections.defaultdict(lambda: collections.defaultdict(list))
        cr._update_database_from_job(jobs[42], database, None)

        tests = database['kernels/simple_diffusion.test'][443499]
        self.assertEqual(len(tests), 5)
        self.assertEqual(tests[1].recipe, '06_Test_-p_3')
        self.assertEqual(tests[1].status, 'OK')
        self.assertEqual(tests[1].caveats, ['recover'])
        self.assertEqual(tests[1].url, SITE)
        self.assertEqual(tests[1].reason, '')

    def testGetCivetResults(self):
        database = cr.get_civet_results(hashes=SHAS, sites=[(SITE, REPO)])
        tests = database['kernels/simple_diffusion.test'][443499]
        self.assertEqual(len(tests), 5)
        self.assertEqual(tests[1].recipe, '06_Test_-p_3')
        self.assertEqual(tests[1].status, 'OK')
        self.assertEqual(tests[1].caveats, ['recover'])
        self.assertEqual(tests[1].url, SITE)
        self.assertEqual(tests[1].reason, '')


if __name__ == '__main__':
    unittest.main(module=__name__, verbosity=2)
