import unittest
import tempfile
import json
import os
from pathlib import Path
from unittest.mock import patch, MagicMock
from GitHubAPI import GitHubAPI


def fake_read_text(self):
    """Fake read_text method for Path objects based on the filename."""
    if self.name == "query.gql.in":
        # A dummy query template; the placeholders correspond to:
        # end_cursor, num_discussion, num_comment, num_reply.
        return "QUERY_CURSOR: {} DISCUSSIONS: {} COMMENTS: {} REPLIES: {}"
    elif self.name == "comments_query.gql.in":
        # A dummy comments query template; placeholders:
        # discussion_id, comments_end_cursor, num_comment, num_reply.
        return "COMMENTS_QUERY: discussion {} CURSOR {} COMMENTS: {} REPLIES: {}"
    return ""


class TestGitHubAPI(unittest.TestCase):
    def setUp(self):
        # Create a temporary directory for output files.
        self.temp_dir = tempfile.TemporaryDirectory()
        self.out_dir = self.temp_dir.name

        # Set a dummy GitHub token in the environment (if not already set).
        os.environ["GITHUB_TOKEN"] = "dummy_token"

        # Create an instance with dry_run False by default.
        self.api = GitHubAPI(
            end_point="https://api.github.com/graphql",
            num_discussion=5,
            num_comment=3,
            num_reply=2,
            min_credit=10,
            out_dir=self.out_dir,
            dry_run=False,
        )

    def tearDown(self):
        # Cleanup temporary directory.
        self.temp_dir.cleanup()

    @patch("builtins.print")
    def test_log(self, mock_print):
        # Call log with test parameters.
        begin_cursor = "start_cursor"
        end_cursor = "end_cursor"
        remaining = 120
        has_next_page = True
        self.api.log(begin_cursor, end_cursor, remaining, has_next_page)

        # Check that print was called with the expected strings.
        expected_calls = [
            (("            From: {}".format(begin_cursor),),),
            (("              To: {}".format(end_cursor),),),
            (("Remaining credit: {}".format(remaining),),),
            (("        Has more: {}".format(has_next_page),),),
            (("-" * 79,),),
        ]
        actual_calls = mock_print.call_args_list
        self.assertEqual(len(actual_calls), len(expected_calls))
        for call, expected in zip(actual_calls, expected_calls):
            self.assertEqual(call, expected)

    @patch("GitHubAPI.Path.read_text", new=fake_read_text)
    @patch("builtins.print")
    def test_fetch_data_dry_run(self, mock_print):
        # Set dry_run to True so that no network call is made.
        self.api.dry_run = True
        self.api.fetch_data()

        # Check that the dry run message was printed.
        mock_print.assert_any_call(
            "Dry run: would execute query with cursor {}".format(self.api.end_cursor)
        )

    @patch("GitHubAPI.Path.read_text", new=fake_read_text)
    @patch("requests.post")
    @patch("builtins.print")
    def test_fetch_data_success(self, mock_print, mock_post):
        # Simulate a successful response from the GitHub API.
        fake_response = MagicMock()
        fake_response.status_code = 200
        fake_response.json.return_value = {
            "data": {
                "repository": {
                    "discussions": {
                        "pageInfo": {
                            "hasNextPage": False,
                            "endCursor": "cursor1",
                        }
                    }
                },
                "rateLimit": {"remaining": 150},
            }
        }
        mock_post.return_value = fake_response

        # Call fetch_data; since hasNextPage becomes False in the fake response,
        # the loop should exit after one iteration.
        self.api.fetch_data()

        # Check that a log was printed.
        self.assertTrue(any("Remaining credit:" in call.args[0] for call in mock_print.call_args_list))

        # Check that a file was written in the out_dir.
        # The filename is created from the begin and new end cursor values.
        expected_filename = "{}_{}.json".format("null", '"cursor1"')
        file_path = Path(self.out_dir) / expected_filename
        self.assertTrue(file_path.exists())

    @patch("GitHubAPI.Path.read_text", new=fake_read_text)
    @patch("requests.post")
    @patch("builtins.print")
    def test_fetch_data_error_status(self, mock_print, mock_post):
        # Simulate an error response (e.g. 404) from the GitHub API.
        fake_response = MagicMock()
        fake_response.status_code = 404
        fake_response.text = "Not Found"
        mock_post.return_value = fake_response

        # When an error occurs, the method calls exit().
        with self.assertRaises(SystemExit):
            self.api.fetch_data()

        # Check that the error message was printed.
        mock_print.assert_any_call("Error: 404")


if __name__ == "__main__":
    unittest.main()
