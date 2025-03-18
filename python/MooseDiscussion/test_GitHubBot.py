import unittest
from unittest.mock import patch, MagicMock
from pathlib import Path
from GitHubBot import GitHubBot

class TestGitHubBot(unittest.TestCase):

    def setUp(self):

        self.load_local = True
        self.model_path = Path('../../../../../../LLM/pretrained_models/')
        self.model_name = 'all-MiniLM-L12-v2'
        self.database = Path("testdatabase")
        self.dry_run = False

        self.mock_index = MagicMock()

        # Initialize GitHubBot with dummy data for all tests
        self.bot = GitHubBot(
            db_dir=self.database,
            model_path=self.model_path,
            top_n=5,
            threshold=0.2,
            model_name=self.model_name,
            # load_local needs to be true within INL network because Zscalar block Huggingface online mode
            load_local= self.load_local,
            dry_run=self.dry_run
        )

        # Mock load_database method to return a dummy index
        self.bot.load_database = MagicMock(return_value='dummy_index')

        # Mock generate_solution method to return the expected formatted string
        self.expected_solution = (
            "Here are some previous posts that may relate to your question: \n\n"
            "1. Title: Test Title 1\n"
            "URL: [http://testurl1.com](http://testurl1.com)\n"
            "Similarity: 0.9000\n\n"
            "2. Title: Test Title 2\n"
            "URL: [http://testurl2.com](http://testurl2.com)\n"
            "Similarity: 0.8000\n"
        )
        self.bot.generate_solution = MagicMock(return_value=self.expected_solution)

    @patch('GitHubBot.requests.post')
    @patch('GitHubBot.SimpleVectorStore.from_persist_dir')
    def test_query_response_success(self, mock_from_persist_dir, mock_post):
        # Mock the vector store loading
        mock_from_persist_dir.return_value = MagicMock()

        # Mock the response from GitHub API for the query
        mock_query_response = MagicMock()
        mock_query_response.status_code = 200
        mock_query_response.json.return_value = {
            'data': {
                'repository': {
                    'discussions': {
                        'nodes': [
                            {
                                'id': 'discussion_1',
                                'title': 'Sample Discussion',
                                'author': {'login': 'author1'},
                                'comments': {'nodes': [{'author': {'login': 'MOOSEbot'}}]}
                            }
                        ]
                    }
                }
            }
        }

        # Mock the response from GitHub API for the mutation
        mock_mutation_response = MagicMock()
        mock_mutation_response.status_code = 200
        mock_mutation_response.json.return_value = {
            'data': {
                'addDiscussionComment': {
                    'comment': {
                        'id': 'comment_1'
                    }
                }
            }
        }

        # Sequence the responses for query and mutation
        mock_post.side_effect = [mock_query_response, mock_mutation_response]

        # Call the method to test
        print("Calling query_response method...")
        self.bot.query_response()

        # Check if the methods were called with expected arguments
        # print("Checking if load_database was called...")
        # self.bot.load_database.assert_called_once_with()

        print("Checking if generate_solution was called...")
        self.bot.generate_solution.assert_called_once_with('Sample Discussion', 5, self.mock_index, 0.2)

        # Check if the requests.post was called
        print("Checking if requests.post was called...")
        self.assertTrue(mock_post.called, "requests.post should be called")
        self.assertEqual(mock_post.call_count, 2, "requests.post should be called twice")

        # Print the call arguments to debug
        for call in mock_post.call_args_list:
            print(f"requests.post called with args: {call}")

    @patch('GitHubBot.requests.post')
    @patch('GitHubBot.SimpleVectorStore.from_persist_dir')
    def test_query_response_failure(self, mock_from_persist_dir, mock_post):
        # Mock the vector store loading
        mock_from_persist_dir.return_value = MagicMock()

        # Mock a failed response from GitHub API
        mock_response = MagicMock()
        mock_response.status_code = 500
        mock_response.text = 'Internal Server Error'
        mock_post.return_value = mock_response

        # Capture the print output
        with patch('builtins.print') as mocked_print:
            self.bot.query_response()
            mocked_print.assert_any_call('Request failed with status code: 500')
            mocked_print.assert_any_call('Internal Server Error')

        # Check if the requests.post was called
        self.assertTrue(mock_post.called, "requests.post should be called")


    @patch('GitHubBot.SimpleVectorStore.from_persist_dir')
    def test_generate_solution(self, mock_from_persist_dir):
        # Mock the vector store loading
        mock_from_persist_dir.return_value = MagicMock()

        # Mock the retriever and processor behavior
        self.bot.embed_model = MagicMock()
        retriever = MagicMock()
        retriever.retrieve.return_value = [
            MagicMock(score=0.9, metadata={'title': 'Test Title 1', 'url': 'http://testurl1.com'}),
            MagicMock(score=0.8, metadata={'title': 'Test Title 2', 'url': 'http://testurl2.com'})
        ]
        processor = MagicMock()
        processor.postprocess_nodes.return_value = retriever.retrieve.return_value

        # Mocking the VectorIndexRetriever and SimilarityPostprocessor
        with patch('GitHubBot.VectorIndexRetriever', return_value=retriever):
            with patch('GitHubBot.SimilarityPostprocessor', return_value=processor):
                result = self.bot.generate_solution('Test Title', 5, 'dummy_index', 0.2)

        self.assertEqual(result, self.expected_solution, "The generate_solution method did not return the expected result")

if __name__ == '__main__':
    unittest.main()
