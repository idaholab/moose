import unittest
from unittest.mock import patch, MagicMock, call
import tempfile
import os
from pathlib import Path
import json
from GitHubBot import GitHubBot, Settings

class TestGitHubBot(unittest.TestCase):
    def setUp(self):
        # Create a temporary directory for the "database" folder.
        self.temp_db_dir = tempfile.TemporaryDirectory()
        self.db_dir = Path(self.temp_db_dir.name)
        # Set a dummy GitHub token.
        os.environ["GITHUB_TOKEN"] = "dummy_token"

    def tearDown(self):
        self.temp_db_dir.cleanup()

    @patch("GitHubBot.load_index_from_storage")
    @patch("GitHubBot.StorageContext.from_defaults")
    @patch("GitHubBot.SimpleVectorStore.from_persist_dir")
    @patch("GitHubBot.HuggingFaceEmbedding")
    def test_init_and_load_database(self, mock_embed, mock_from_persist, mock_from_defaults, mock_load_index):
        # Create fake objects for vector store, storage context and index.
        fake_vector_store = MagicMock()
        fake_storage_context = MagicMock()
        fake_index = MagicMock()

        mock_from_persist.return_value = fake_vector_store
        mock_from_defaults.return_value = fake_storage_context
        mock_load_index.return_value = fake_index
        mock_embed.return_value = MagicMock()  # dummy embed model

        # Instantiate the bot.
        bot = GitHubBot(db_dir=self.db_dir, top_n=3, threshold=0.5,
                        model_name="dummy_model", dry_run=True)
        # load_database is called during initialization.
        self.assertEqual(bot.index, fake_index)
        self.assertIsNotNone(bot.embed_model)
        # Verify that the Settings.embed_model was set.
        self.assertEqual(Settings.embed_model, bot.embed_model)

    @patch("GitHubBot.load_index_from_storage")
    @patch("GitHubBot.StorageContext.from_defaults")
    @patch("GitHubBot.SimpleVectorStore.from_persist_dir")
    def test_load_database(self, mock_from_persist, mock_from_defaults, mock_load_index):
        fake_vector_store = MagicMock()
        fake_storage_context = MagicMock()
        fake_index = MagicMock()

        mock_from_persist.return_value = fake_vector_store
        mock_from_defaults.return_value = fake_storage_context
        mock_load_index.return_value = fake_index

        # Instantiate the bot (patching out the embedding to avoid side effects).
        with patch("GitHubBot.HuggingFaceEmbedding"):
            bot = GitHubBot(db_dir=self.db_dir, top_n=3, threshold=0.5,
                            model_name="dummy_model", dry_run=True)
        # Call load_database explicitly.
        index = bot.load_database(self.db_dir)
        self.assertEqual(index, fake_index)
        mock_from_persist.assert_called_with(self.db_dir)
        mock_from_defaults.assert_called_with(vector_store=fake_vector_store, persist_dir=self.db_dir)
        mock_load_index.assert_called_with(storage_context=fake_storage_context)

    @patch("GitHubBot.VectorIndexRetriever")
    @patch("GitHubBot.SimilarityPostprocessor")
    def test_generate_solution(self, mock_postprocessor, mock_retriever):
        # Set up a fake retriever instance.
        fake_retriever_instance = MagicMock()
        fake_postprocessor_instance = MagicMock()

        # Create two dummy nodes.
        fake_node1 = MagicMock()
        fake_node1.metadata = {'title': 'Test Title 1', 'url': 'http://example.com/1'}
        fake_node1.score = 0.95

        fake_node2 = MagicMock()
        fake_node2.metadata = {'title': 'Test Title 2', 'url': 'http://example.com/2'}
        fake_node2.score = 0.85

        fake_retriever_instance.retrieve.return_value = [fake_node1, fake_node2]
        fake_postprocessor_instance.postprocess_nodes.return_value = [fake_node1, fake_node2]

        mock_retriever.return_value = fake_retriever_instance
        mock_postprocessor.return_value = fake_postprocessor_instance

        # Create the bot instance (patching out database and model loading).
        with patch("GitHubBot.load_index_from_storage"), patch("GitHubBot.HuggingFaceEmbedding"):
            bot = GitHubBot(db_dir=self.db_dir, top_n=2, threshold=0.5,
                            model_name="dummy_model", dry_run=True)
        # Call generate_solution.
        result = bot.generate_solution("Test query", top_n=2, index=bot.index, threshold=0.5)
        # Verify the output contains expected texts.
        self.assertIn("Here are some previous posts", result)
        self.assertIn("1. Title: Test Title 1", result)
        self.assertIn("URL: [http://example.com/1](http://example.com/1)", result)
        self.assertIn("Similarity: 0.95", result)
        self.assertIn("2. Title: Test Title 2", result)
        self.assertIn("URL: [http://example.com/2](http://example.com/2)", result)
        self.assertIn("Similarity: 0.85", result)
        # Ensure that the retriever's retrieve and the postprocessor's postprocess_nodes were called.
        fake_retriever_instance.retrieve.assert_called()
        fake_postprocessor_instance.postprocess_nodes.assert_called()


    @patch("GitHubBot.requests.post")
    @patch("GitHubBot.GitHubBot.generate_solution")
    def test_query_response_dry_run(self, mock_generate_solution, mock_requests_post):
        # Set up a fake discussion.
        fake_discussion = {
            'id': 'disc1',
            'title': 'Discussion 1',
            'author': {'login': 'user1'},
            'comments': {'nodes': []}  # No comments so that reply proceeds.
        }
        fake_response_data = {
            'data': {
                'repository': {
                    'discussions': {
                        'nodes': [fake_discussion]
                    }
                }
            }
        }
        fake_response = MagicMock()
        fake_response.status_code = 200
        fake_response.json.return_value = fake_response_data

        mock_requests_post.return_value = fake_response
        mock_generate_solution.return_value = "Fake solution"

        # Create bot in dry run mode.
        with patch("GitHubBot.load_index_from_storage"), patch("GitHubBot.HuggingFaceEmbedding"):
            bot = GitHubBot(db_dir=self.db_dir, top_n=2, threshold=0.5,
                            model_name="dummy_model", dry_run=True)
        # Capture printed output.
        with patch("builtins.print") as mock_print:
            bot.query_response()
            # Check that the dry run message was printed.
            calls = [args[0] for args, _ in mock_print.call_args_list]
            self.assertTrue(any("Dry run mode: Would have replied to discussion: Discussion 1" in s for s in calls))
        # In dry run mode, only one POST (for fetching discussions) should occur.
        self.assertEqual(mock_requests_post.call_count, 1)

    @patch("GitHubBot.requests.post")
    @patch("GitHubBot.GitHubBot.generate_solution")
    def test_query_response_non_dry_run_success(self, mock_generate_solution, mock_requests_post):
        # Test non-dry run mode where posting a comment is successful.
        fake_discussion = {
            'id': 'disc2',
            'title': 'Discussion 2',
            'author': {'login': 'user2'},
            'comments': {'nodes': []}
        }
        fake_query_response_data = {
            'data': {
                'repository': {
                    'discussions': {
                        'nodes': [fake_discussion]
                    }
                }
            }
        }
        fake_mutation_response_data = {
            'data': {
                'addDiscussionComment': {
                    'comment': {'id': 'comment123'}
                }
            }
        }
        # First POST call returns the discussion.
        fake_query_response = MagicMock()
        fake_query_response.status_code = 200
        fake_query_response.json.return_value = fake_query_response_data

        # Second POST call returns the mutation result.
        fake_mutation_response = MagicMock()
        fake_mutation_response.status_code = 200
        fake_mutation_response.json.return_value = fake_mutation_response_data

        mock_requests_post.side_effect = [fake_query_response, fake_mutation_response]
        mock_generate_solution.return_value = "Fake solution for Discussion 2"

        with patch("GitHubBot.load_index_from_storage"), patch("GitHubBot.HuggingFaceEmbedding"):
            bot = GitHubBot(db_dir=self.db_dir, top_n=2, threshold=0.5,
                            model_name="dummy_model", dry_run=False)
        with patch("builtins.print") as mock_print:
            bot.query_response()
            # Verify that a success message was printed.
            calls = [args[0] for args, _ in mock_print.call_args_list]
            self.assertTrue(any("Successfully replied to discussion: Discussion 2 (Comment ID: comment123)" in s for s in calls))
        self.assertEqual(mock_requests_post.call_count, 2)

    @patch("GitHubBot.requests.post")
    def test_query_response_fetch_failure(self, mock_requests_post):
        # Test when the initial discussion fetch fails (non-200 response).
        fake_response = MagicMock()
        fake_response.status_code = 500
        fake_response.text = "Internal Server Error"
        mock_requests_post.return_value = fake_response

        with patch("builtins.print") as mock_print, \
             patch("GitHubBot.load_index_from_storage"), \
             patch("GitHubBot.HuggingFaceEmbedding"):
            bot = GitHubBot(db_dir=self.db_dir, top_n=2, threshold=0.5,
                            model_name="dummy_model", dry_run=True)
            bot.query_response()
            calls = [args[0] for args, _ in mock_print.call_args_list]
            self.assertTrue(any("Request failed with status code: 500" in s for s in calls))

    @patch("GitHubBot.requests.post")
    @patch("GitHubBot.GitHubBot.generate_solution")
    def test_query_response_mutation_failure(self, mock_generate_solution, mock_requests_post):
        # Test non-dry run mode where posting a comment (mutation) fails.
        fake_discussion = {
            'id': 'disc3',
            'title': 'Discussion 3',
            'author': {'login': 'user3'},
            'comments': {'nodes': []}
        }
        fake_query_response_data = {
            'data': {
                'repository': {
                    'discussions': {
                        'nodes': [fake_discussion]
                    }
                }
            }
        }
        fake_query_response = MagicMock()
        fake_query_response.status_code = 200
        fake_query_response.json.return_value = fake_query_response_data

        # Simulate mutation failure.
        fake_mutation_response = MagicMock()
        fake_mutation_response.status_code = 400
        fake_mutation_response.text = "Bad Request"

        mock_requests_post.side_effect = [fake_query_response, fake_mutation_response]
        mock_generate_solution.return_value = "Fake solution for Discussion 3"

        with patch("GitHubBot.load_index_from_storage"), patch("GitHubBot.HuggingFaceEmbedding"):
            bot = GitHubBot(db_dir=self.db_dir, top_n=2, threshold=0.5,
                            model_name="dummy_model", dry_run=False)
        with patch("builtins.print") as mock_print:
            bot.query_response()
            # Verify that a failure message was printed.
            printed = [args[0] for args, _ in mock_print.call_args_list]
            self.assertTrue(any("Failed to add comment to discussion: Discussion 3" in s for s in printed))
        self.assertEqual(mock_requests_post.call_count, 2)

if __name__ == "__main__":
    unittest.main()
