# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

"""Test TestHarness.resultsstore.resultsreader.ResultsReader."""

# import json
import os
import unittest
from copy import deepcopy
from dataclasses import asdict
from tempfile import NamedTemporaryFile
from unittest.mock import patch

import pytest
import yaml
from bson.objectid import ObjectId
from mock import MagicMock
from TestHarness.resultsstore.auth import Authentication
from TestHarness.resultsstore.reader import ResultsReader, ResultsReaderContextManager
from TestHarness.resultsstore.storedresult import StoredResult
from TestHarness.tests.resultsstore.common import (
    GOLD_DATABASE_NAME,
    GOLD_RESULTS,
    OBJECT_ID,
    TEST_DATABASE_NAME,
    FakeMongoClient,
    FakeMongoDatabase,
    FakeMongoFind,
    ResultsStoreTestCase,
    random_git_sha,
)
from TestHarness.tests.resultsstore.test_auth import DEFAULT_AUTH, get_auth_env

# Authentication available in the environment, if any
AUTH = ResultsReader.load_authentication()
# Whether or not authentication is available
HAS_AUTH = AUTH is not None

DATABASE_NAME = "foodb"


class TestResultsReader(ResultsStoreTestCase):
    """Test TestHarness.resultsstore.reader.ResultsReader."""

    def test_init_client(self):
        """Test __init__() with a client passed in."""
        client = FakeMongoClient()
        with patch.dict(os.environ, {}, clear=True):
            reader = ResultsReader(DATABASE_NAME, client)
        self.assertEqual(reader._database_name, DATABASE_NAME)
        self.assertTrue(reader.check)
        self.assertEqual(reader._timeout, 5.0)
        self.assertEqual(id(reader._client), id(client))
        self.assertIsNone(reader._database)
        self.assertIsNone(reader._authentication)

    def test_init_env_auth(self):
        """Test __init__() with environment authentication."""
        auth_env = get_auth_env("RESULTS_READER")
        with patch.dict(os.environ, auth_env, clear=True):
            reader = ResultsReader(DATABASE_NAME)
        self.assertEqual(asdict(reader._authentication), {**DEFAULT_AUTH, "port": None})
        self.assertIsNone(reader._client)

    def test_init_file_auth(self):
        """Test __init__() with file authentication."""
        with NamedTemporaryFile() as auth_file:
            with open(auth_file.name, "w") as f:
                yaml.safe_dump(DEFAULT_AUTH, f)

            auth_env = {"RESULTS_READER_AUTH_FILE": auth_file.name}
            with patch.dict(os.environ, auth_env, clear=True):
                reader = ResultsReader(DATABASE_NAME)

        self.assertEqual(asdict(reader._authentication), {**DEFAULT_AUTH, "port": None})
        self.assertIsNone(reader._client)

    def test_init_auth(self):
        """Test __init__() with authentication by variable."""
        auth = Authentication(**DEFAULT_AUTH)
        reader = ResultsReader(DATABASE_NAME, authentication=auth)
        self.assertEqual(id(reader._authentication), id(auth))

    def test_init_no_auth(self):
        """Test __init__() without any authentication."""
        with (
            patch.dict(os.environ, {}, clear=True),
            self.assertRaisesRegex(
                ValueError,
                "Must specify either 'client', 'authentication' or set",
            ),
        ):
            ResultsReader(DATABASE_NAME)

    def test_init_bad_client(self):
        """Test __init__() with an invalid client."""
        with self.assertRaisesRegex(TypeError, "Invalid type for 'client'"):
            ResultsReader(DATABASE_NAME, client="foo")

    def test_del(self):
        """Test that __del__() calls close()."""
        reader = ResultsReader(DATABASE_NAME, FakeMongoClient())
        reader.close = MagicMock()
        reader.__del__()
        reader.close.assert_called_once()

    def test_enter(self):
        """Test __enter__() returning the context manager."""
        reader = ResultsReader(DATABASE_NAME, FakeMongoClient())
        result = reader.__enter__()
        self.assertIsInstance(result, ResultsReaderContextManager)
        self.assertEqual(result.reader, reader)

    def test_exit(self):
        """Test __exit__() calling close."""
        reader = ResultsReader(DATABASE_NAME, FakeMongoClient())
        reader.close = MagicMock()
        reader.__exit__(None, None, None)
        reader.close.assert_called_once()

    def test_load_authentication(self):
        """Test load_authentication calling the underlying load_authentication."""
        reader = ResultsReader(DATABASE_NAME, FakeMongoClient())
        with patch(
            "TestHarness.resultsstore.reader.load_authentication"
        ) as patch_load_authentication:
            reader.load_authentication()
        patch_load_authentication.assert_called_once_with("RESULTS_READER")

    def test_get_client(self):
        """Test get_client()."""
        auth_env = get_auth_env("RESULTS_READER")
        with patch.dict(os.environ, auth_env, clear=True):
            reader = ResultsReader(DATABASE_NAME)
        self.assertIsNone(reader._client)

        # Builds the first time
        with patch(
            "TestHarness.resultsstore.reader.pymongo.MongoClient",
            return_value=FakeMongoClient(),
        ) as patch_mongo_client:
            client = reader.get_client()
            self.assertIsInstance(client, FakeMongoClient)
        patch_mongo_client.assert_called_once_with(
            DEFAULT_AUTH["host"],
            username=DEFAULT_AUTH["username"],
            password=DEFAULT_AUTH["password"],
            port=None,
            timeoutMS=reader._timeout * 1000,
        )

        # Not built after the first time
        with patch(
            "TestHarness.resultsstore.reader.pymongo.MongoClient",
        ) as patch_mongo_client:
            client_again = reader.get_client()
        self.assertEqual(id(client), id(client_again))
        patch_mongo_client.assert_not_called()

    def test_get_database(self):
        """Test get_database()."""
        client = FakeMongoClient()
        reader = ResultsReader(DATABASE_NAME, client)
        database = FakeMongoDatabase()

        # Gets the first time
        with patch.object(
            client, "get_database", return_value=database
        ) as patch_get_database:
            got_database = reader.get_database()
        self.assertEqual(id(got_database), id(database))
        patch_get_database.assert_called_once_with(DATABASE_NAME)

        # Second time uses the same value
        with patch.object(client, "get_database") as patch_get_database:
            got_database = reader.get_database()
        self.assertEqual(id(got_database), id(database))
        patch_get_database.assert_not_called()

    def test_close_without_client(self):
        """Test close() doing nothing if the client isn't set."""
        auth_env = get_auth_env("RESULTS_READER")
        with patch.dict(os.environ, auth_env, clear=True):
            reader = ResultsReader(DATABASE_NAME)
        reader.close()

    def test_close_with_client(self):
        """Test close() closing the client if set."""
        client = FakeMongoClient()
        reader = ResultsReader(DATABASE_NAME, client)
        with patch.object(client, "close") as patch_close:
            reader.close()
        patch_close.assert_called_once()

    def test_close_with_client_no_close(self):
        """Test close() not closing the client of close_client=False."""
        client = FakeMongoClient()
        reader = ResultsReader(DATABASE_NAME, client, close_client=False)
        with patch.object(client, "close") as patch_close:
            reader.close()
        patch_close.assert_not_called()

    def test_find_results_no_limit(self):
        """Test _find_results() without limit."""
        client = FakeMongoClient()
        reader = ResultsReader(DATABASE_NAME, client)

        database = client.get_database(DATABASE_NAME)
        found_results = [{"foo": "bar"}, {"baz": "bang"}]
        find_filter = {"bad": "filter"}
        with patch.object(
            database.results, "find", return_value=FakeMongoFind(found_results)
        ) as patch_find:
            results = reader._find_results(find_filter, None)

        self.assertEqual(results, found_results)
        patch_find.assert_called_once_with(
            find_filter, {"tests": 0}, sort=reader.mongo_sort_id
        )

    def test_find_results_limit(self):
        """Test _find_results() with a limit."""
        client = FakeMongoClient()
        reader = ResultsReader(DATABASE_NAME, client)

        database = client.get_database(DATABASE_NAME)
        found_results = [{"foo": "bar"}, {"baz": "bang"}]
        find_filter = {"bad": "filter"}
        find_limit = 10
        with patch.object(
            database.results, "find", return_value=FakeMongoFind(found_results)
        ) as patch_find:
            results = reader._find_results(find_filter, find_limit)

        self.assertEqual(results, found_results)
        patch_find.assert_called_once_with(
            find_filter, {"tests": 0}, sort=reader.mongo_sort_id, limit=find_limit
        )

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_find_results_live(self):
        """Test _find_results() with the live database."""
        with ResultsReader(TEST_DATABASE_NAME, authentication=AUTH) as ctx:
            docs = ctx.reader._find_results({}, limit=2)

        # Should have two documents
        self.assertEqual(len(docs), 2)

        # Should be valid documents
        [StoredResult(doc) for doc in docs]

    def test_aggregate_results(self):
        """Test _aggregate_results()."""
        client = FakeMongoClient()
        reader = ResultsReader(DATABASE_NAME, client)

        database = client.get_database(DATABASE_NAME)
        found_results = [{"foo": "bar"}, {"baz": "bang"}]
        pipeline = [{"bad": "pipeline"}]
        with patch.object(
            database.results, "aggregate", return_value=FakeMongoFind(found_results)
        ) as patch_find:
            results = reader._aggregate_results(pipeline)

        self.assertEqual(results, found_results)
        patch_find.assert_called_once_with(pipeline)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_aggregate_results_live(self):
        """Test _aggregate_results() with the live database."""
        with ResultsReader(TEST_DATABASE_NAME, authentication=AUTH) as ctx:
            docs = ctx.reader._aggregate_results([{"$limit": 2}])

        # Should have two documents
        self.assertEqual(len(docs), 2)

        # Should be valid documents
        [StoredResult(doc) for doc in docs]

    def test_build_result(self):
        """Test build_result()."""
        # Setup result for loading
        data = self.get_result_data()

        reader = ResultsReader(DATABASE_NAME, FakeMongoClient(), check=True)
        self.assertEqual(len(reader._results), 0)

        # Add an entry; doesn't exist yet
        stored_result = reader._build_result(data)
        self.assertIsInstance(stored_result, StoredResult)
        self.assertEqual(len(reader._results), 1)
        self.assertEqual(id(reader._results[OBJECT_ID]), id(stored_result))

        # Call again, should now exist
        stored_result_again = reader._build_result(data)
        self.assertEqual(id(stored_result), id(stored_result_again))

    def test_build_result_failed(self):
        """Test build_result() raising during the build."""
        reader = ResultsReader(DATABASE_NAME, FakeMongoClient(), check=True)
        bad_data = {"_id": OBJECT_ID}
        with self.assertRaisesRegex(
            ValueError, f"Failed to build result _id={OBJECT_ID}"
        ):
            reader._build_result(bad_data)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_build_result_live(self):
        """Test build_result() with the live database."""
        # Test the most recent result
        with ResultsReader(TEST_DATABASE_NAME, authentication=AUTH, check=True) as ctx:
            reader = ctx.reader
            docs = reader._find_results({}, limit=1)
            self.assertEqual(len(docs), 1)
            reader._build_result(docs[0])

    def test_last_push_results_iterator_no_num(self):
        """
        Test last_push_results_iterator() with no num set (unlimited).

        Get two entries, coming from three where one has the same event ID.
        """
        data = [deepcopy(self.get_result_data(pr=False)) for _ in range(3)]
        for v in reversed(data):
            v["_id"] = ObjectId()

        data_index = 0
        pipelines = []

        def new_aggregate_results(pipeline):
            nonlocal pipelines
            pipelines.append(pipeline)

            nonlocal data_index
            data_index += 1
            if data_index < 4:
                return [data[data_index - 1]]
            return []

        reader = ResultsReader(DATABASE_NAME, FakeMongoClient())
        with patch.object(reader, "_aggregate_results", new=new_aggregate_results):
            results = [v for v in reader.latest_push_results_iterator()]

        # Should have exactly three results
        self.assertEqual(len(results), 3)
        # Should have found the last
        self.assertTrue(reader._found_final_push_event)

        # Should pull after last ID after first case
        base_match = {"event_cause": {"$ne": "pr"}}
        expect_matches = [
            {**base_match},
            {"_id": {"$lt": data[0]["_id"]}, **base_match},
            {"_id": {"$lt": data[1]["_id"]}, **base_match},
            {"_id": {"$lt": data[2]["_id"]}, **base_match},
        ]
        self.assertEqual(expect_matches, [v[0]["$match"] for v in pipelines])

        # We found the last one, so nothing else to find (won't call again)
        with patch.object(reader, "_aggregate_results") as patch_aggregate_results:
            results = [v for v in reader.latest_push_results_iterator()]
        patch_aggregate_results.assert_not_called()

    def test_last_push_results_iterator_num(self):
        """Test last_push_results_iterator() with num set."""
        reader = ResultsReader(DATABASE_NAME, FakeMongoClient())
        data = [deepcopy(self.get_result_data(pr=False)) for _ in range(4)]
        for v in reversed(data):
            v["_id"] = ObjectId()

        data_it = iter(data)
        aggregate_calls = 0

        def new_aggregate_results(_):
            nonlocal data_it
            nonlocal aggregate_calls
            try:
                return [next(data_it)]
            except StopIteration:
                return []
            finally:
                aggregate_calls += 1

        # Get the first two
        with patch.object(reader, "_aggregate_results", new=new_aggregate_results):
            it = reader.latest_push_results_iterator(2)
            results = [next(it) for _ in range(2)]
        self.assertEqual(len(results), 2)
        self.assertEqual(aggregate_calls, 2)
        self.assertFalse(reader._found_final_push_event)

        # Get two again, which won't get anything
        with patch.object(reader, "_aggregate_results") as patch_aggregate_results:
            it = reader.latest_push_results_iterator(2)
            results = [next(it) for _ in range(2)]
        self.assertEqual(len(results), 2)
        patch_aggregate_results.assert_not_called()
        self.assertFalse(reader._found_final_push_event)

        # Get two more on top of the original 2
        with patch.object(reader, "_aggregate_results", new=new_aggregate_results):
            it = reader.latest_push_results_iterator(2)
            results = [next(it) for _ in range(4)]
        self.assertEqual(len(results), 4)
        self.assertEqual(aggregate_calls, 4)
        self.assertFalse(reader._found_final_push_event)

        # Get all of them, which should query one last time (with nothing)
        with patch.object(reader, "_aggregate_results", new=new_aggregate_results):
            results = [v for v in reader.latest_push_results_iterator()]
        self.assertEqual(len(results), 4)
        self.assertEqual(aggregate_calls, 5)
        self.assertTrue(reader._found_final_push_event)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_lastest_push_results_iterator(self):
        """Test last_push_results_iterator() with a live database."""
        num = 50
        with ResultsReader(TEST_DATABASE_NAME, authentication=AUTH) as ctx:
            reader = ctx.reader
            it = reader.latest_push_results_iterator(num - 1)
            results = [next(it) for _ in range(num)]

        # Should have exactly the number as expected
        self.assertEqual(len(results), num)
        # And should have loaded no more
        self.assertEqual(len(reader._latest_push_results), num)
        # And should have no duplicate events
        self.assertEqual(len(set([r.event_id for r in results])), num)

    def test_get_latest_push_results(self):
        """Test get_latest_push_results()."""
        reader = ResultsReader(DATABASE_NAME, FakeMongoClient())

        data = [deepcopy(self.get_result_data()) for _ in range(5)]
        results = [reader._build_result(v) for v in data]

        self._stdout_patcher.stop()

        def fake_it(num):
            return iter(results)

        # Just get one
        with patch.object(reader, "latest_push_results_iterator", new=fake_it):
            collection = reader.get_latest_push_results(1)
        assert collection is not None
        self.assertEqual(len(collection.results), 1)
        self.assertEqual(collection._database_getter, reader.get_database)
        self.assertEqual(id(collection.results[0]), id(results[0]))

        # Now two
        with patch.object(reader, "latest_push_results_iterator", new=fake_it):
            collection = reader.get_latest_push_results(2)
        assert collection is not None
        self.assertEqual(len(collection.results), 2)
        self.assertEqual(id(collection.results[0]), id(results[0]))
        self.assertEqual(id(collection.results[1]), id(results[1]))

        # And then all (oversized)
        with patch.object(reader, "latest_push_results_iterator", new=fake_it):
            collection = reader.get_latest_push_results(10)
        assert collection is not None
        self.assertEqual(len(collection.results), 5)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_get_latest_push_results_live(self):
        """Test get_latest_push_results() with a live database."""
        with ResultsReader(TEST_DATABASE_NAME, authentication=AUTH) as ctx:
            reader = ctx.reader
            collection = reader.get_latest_push_results(10)
        assert collection is not None
        self.assertEqual(len(collection.results), 10)
        for result in collection.results:
            self.assertNotEqual(result.event_cause, "pr")

    def test_get_latest_push_result_id(self):
        """Test get_latest_push_result_id()."""
        client = FakeMongoClient()
        reader = ResultsReader(DATABASE_NAME, client)
        database = client.get_database(DATABASE_NAME)

        # ID found
        id = ObjectId()
        with patch.object(
            database.results, "find_one", return_value={"_id": id}
        ) as patch_find_one:
            found_id = reader.get_latest_push_result_id()
        self.assertEqual(id, found_id)
        patch_find_one.assert_called_once_with(
            {"event_cause": {"$ne": "pr"}}, {"_id": 1}, sort=[("_id", -1)]
        )

        # ID not found
        id = ObjectId()
        with patch.object(database.results, "find_one", return_value=None):
            found_id = reader.get_latest_push_result_id()
        self.assertIsNone(found_id)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_get_latest_push_result_id_live(self):
        """Test get_latest_push_result_id() with a live database."""
        with ResultsReader(TEST_DATABASE_NAME, authentication=AUTH) as ctx:
            reader = ctx.reader

            id = reader.get_latest_push_result_id()
            collection = reader.get_latest_push_results(1)
            self.assertEqual(len(collection.results), 1)
            self.assertEqual(collection.results[0].id, id)

    def run_test_get_cached_result(self, method_index: str, index: str, value):
        """Test get_cached_result() for the given index."""
        reader = ResultsReader(DATABASE_NAME, FakeMongoClient())
        data = self.get_result_data()
        data[index] = value
        method = getattr(reader, f"get_{method_index}_result")

        # Build the first time
        with patch.object(
            reader, "_find_results", return_value=[data]
        ) as patch_find_results:
            collection = method(value)
        patch_find_results.assert_called_once_with({index: {"$eq": value}}, limit=1)

        # Check built state
        self.assertIsNotNone(collection)
        cache = reader._cached_results[index]
        self.assertEqual(len(cache), 1)
        self.assertIn(value, cache)

        # Now exists the second time
        with patch.object(
            reader, "_find_results", return_value=[data]
        ) as patch_find_results:
            collection_again = method(value)
        patch_find_results.assert_not_called()
        assert collection_again is not None
        self.assertEqual(id(collection_again.result), id(collection.result))

    def test_get_event_result(self):
        """Test get_event_result()."""
        self.run_test_get_cached_result("event", "event_id", 1234)

    def test_get_pr_result(self):
        """Test get_pr_result()."""
        self.run_test_get_cached_result("pr", "pr_num", 1234)

    def test_get_commit_result(self):
        """Test get_commit_result()."""
        self.run_test_get_cached_result("commit", "event_sha", random_git_sha())

    def test_get_id_result(self):
        """Test get_id_result()."""
        self.run_test_get_cached_result("id", "_id", ObjectId())

    def run_test_get_cached_result_none(self, method_index: str, index: str, value):
        """Test get_cached_result() for the given index when none found."""
        reader = ResultsReader(DATABASE_NAME, FakeMongoClient())
        cache = reader._cached_results[index]
        method = getattr(reader, f"get_{method_index}_result")

        # Doesn't exist in the cache
        self.assertNotIn(value, cache)

        # Find it the first time, as none
        with patch.object(
            reader, "_find_results", return_value=[]
        ) as patch_find_results:
            collection = method(value)
        self.assertIsNone(collection)
        patch_find_results.assert_called_once()

        # Should now exist in the cache as none
        self.assertIn(value, cache)
        self.assertIsNone(cache[value])

        # Find it again, shouldn't search (is in cache)
        with patch.object(reader, "_find_results") as patch_find_results:
            collection = method(value)
        patch_find_results.assert_not_called()
        self.assertIsNone(collection)

    def test_get_event_result_none(self):
        """Test get_event_result() when nothing is found."""
        self.run_test_get_cached_result_none("event", "event_id", 1234)

    def test_get_pr_result_none(self):
        """Test get_pr_result() when nothing is found."""
        self.run_test_get_cached_result_none("pr", "pr_num", 1234)

    def test_get_commit_result_none(self):
        """Test get_commit_result() when nothing is found."""
        self.run_test_get_cached_result_none("commit", "event_sha", random_git_sha())

    def test_get_id_result_none(self):
        """Test get_id_result() when nothing is found."""
        self.run_test_get_cached_result_none("id", "_id", ObjectId())

    def run_test_get_cached_result_live(
        self, method_index: str, index: str, filter: dict
    ):
        """Test get_cached_result() for the given index live."""
        with ResultsReader(TEST_DATABASE_NAME) as ctx:
            reader = ctx.reader
            method = getattr(reader, f"get_{method_index}_result")

            # Find a valid value
            docs = reader._find_results(filter, limit=1)
            self.assertEqual(len(docs), 1)
            value = docs[0]["_id" if index == "id" else index]

            collection = method(value)
            assert collection is not None
            self.assertEqual(getattr(collection.result, index), value)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_get_event_result_live(self):
        """Test get_event_result() with a live database."""
        # Test the latest event
        self.run_test_get_cached_result_live("event", "event_id", {})

        # Test the gold events
        for gold_result in GOLD_RESULTS:
            event_id = gold_result.event_id
            if event_id is not None:
                with ResultsReader(GOLD_DATABASE_NAME, authentication=AUTH) as ctx:
                    reader = ctx.reader
                    collection = reader.get_event_result(event_id)
                    assert collection is not None
                    result = collection.result

                    self.assertEqual(result.id, gold_result.id)
                    self.assertEqual(result.civet_version, gold_result.civet_version)
                    self.assertEqual(result.event_sha, gold_result.event_sha)
                    self.assertEqual(result.event_id, event_id)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_get_pr_result_live(self):
        """Test get_pr_result() with a live database."""
        self.run_test_get_cached_result_live(
            "pr", "pr_num", {"event_cause": {"$eq": "pr"}}
        )

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_get_commit_result_live(self):
        """Test get_commit_result() with a live database."""
        # Test with the latest event
        self.run_test_get_cached_result_live("commit", "event_sha", {})

        # Test the gold events
        for gold_result in GOLD_RESULTS:
            event_sha = gold_result.event_sha
            with ResultsReader(GOLD_DATABASE_NAME, authentication=AUTH) as ctx:
                reader = ctx.reader
                collection = reader.get_commit_result(event_sha)
                assert collection is not None

                result = collection.result
                self.assertEqual(result.id, gold_result.id)
                self.assertEqual(result.civet_version, gold_result.civet_version)
                self.assertEqual(result.event_sha, event_sha)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_get_id_result_live(self):
        """Test get_id_result() with a live database."""
        self.run_test_get_cached_result_live("id", "id", {})

    def run_test_get_cached_result_none_live(
        self, method_index: str, value, filter: dict
    ):
        """Test get_event_result() with no result and a live database."""
        with ResultsReader(TEST_DATABASE_NAME) as ctx:
            reader = ctx.reader

            # Make sure it doesn't actually exist
            docs = reader._find_results(filter, limit=1)
            self.assertEqual(len(docs), 0)

            method = getattr(reader, f"get_{method_index}_result")
            collection = method(value)
            self.assertIsNone(collection)

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_get_event_result_none_live(self):
        """Test get_event_result() with a live database and no such event."""
        event_id = 1
        self.run_test_get_cached_result_none_live(
            "event", event_id, {"event_id": {"$eq": event_id}}
        )

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_get_pr_result_none_live(self):
        """Test get_pr_result() with a live database and no such PR."""
        pr_num = 1
        self.run_test_get_cached_result_none_live(
            "pr", pr_num, {"pr_num": {"$eq": pr_num}}
        )

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_get_commit_none_live(self):
        """Test get_commit_result() with a live database and no such commit."""
        commit = random_git_sha()
        self.run_test_get_cached_result_none_live(
            "commit", commit, {"event_sha": {"$eq": commit}}
        )

    @pytest.mark.live_db
    @unittest.skipUnless(HAS_AUTH, "Reader authentication unavailable")
    def test_get_id_none_live(self):
        """Test get_id_result() with a live database and no such ID."""
        result_id = ObjectId()
        self.run_test_get_cached_result_none_live(
            "id", result_id, {"_id": {"$eq": result_id}}
        )
