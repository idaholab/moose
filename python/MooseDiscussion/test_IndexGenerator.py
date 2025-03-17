import unittest
from unittest.mock import patch, MagicMock
from pathlib import Path
import tempfile
import json
from llama_index.core import Document
from llama_index.core.node_parser import SemanticSplitterNodeParser
from llama_index.core.schema import TextNode
from llama_index.embeddings.huggingface import HuggingFaceEmbedding
from llama_index.core import VectorStoreIndex, StorageContext, Settings
from llama_index.core.storage.docstore import SimpleDocumentStore
from llama_index.core.storage.index_store import SimpleIndexStore
from llama_index.core.vector_stores import SimpleVectorStore
from sentence_transformers import SentenceTransformer
from IndexGenerator import IndexGenerator

class TestIndexGenerator(unittest.TestCase):

    def setUp(self):
        self.temp_dir = tempfile.TemporaryDirectory()
        self.mock_data_path = Path(self.temp_dir.name) / "mock_data.json"

        # Example mock data
        mock_data = {
            "discussions": {
                "edges": [
                    {
                        "node": {
                            "title": "Test Title",
                            "url": "http://example.com",
                            "bodyText": "Test body text"
                        }
                    }
                ]
            }
        }

        # Write mock data to a temporary file
        with open(self.mock_data_path, 'w') as f:
            json.dump(mock_data, f)


        self.index_generator = IndexGenerator(
            load_local=True,
            model_path=Path('../../../../../../LLM/pretrained_models/'),
            model_name="all-MiniLM-L12-v2",
            show_progress=False,
            rawdata=str(self.mock_data_path),
            database=Path("testdatabase"),
            dry_run=False,
        )

    def tearDown(self):
        self.temp_dir.cleanup()


    def test_get_post_content(self):
        post = {
            "title": "Test Title",
            "bodyText": "Test body text",
            "comments": {
                "edges": [
                    {"node": {"bodyText": "Test comment"}}
                ]
            }
        }
        content = self.index_generator.get_post_content(post)
        expected_content = "Test Title\nTest body text\nTest comment\n"
        self.assertEqual(content, expected_content)

    def test_get_comment_content(self):
        comment = {
            "bodyText": "Test comment",
            "replies": {
                "edges": [
                    {"node": {"bodyText": "Test reply"}}
                ]
            }
        }
        content = self.index_generator.get_comment_content(comment)
        expected_content = "Test comment\nTest reply\n"
        self.assertEqual(content, expected_content)


    @patch("builtins.open", new_callable=unittest.mock.mock_open, read_data='{"discussions": {"edges": [{"node": {"title": "Test Title", "url": "http://example.com", "bodyText": "Test body text"}}]}}')
    def test_my_file_reader_load_data(self, mock_open):
        file_reader = self.index_generator.MyFileReader(self.index_generator.get_post_content)
        documents = file_reader.load_data("fake_file.json")
        self.assertIsInstance(documents, list)
        self.assertIsInstance(documents[0], Document)
        self.assertEqual(documents[0].text, "Test Title\nTest body text\n")
        self.assertEqual(documents[0].metadata, {"title": "Test Title", "url": "http://example.com"})

    @patch("pathlib.Path.joinpath", return_value=Path('../../../../../../LLM/pretrained_models/all-MiniLM-L12-v2'))
    @patch("llama_index.embeddings.huggingface.HuggingFaceEmbedding")
    @patch("sentence_transformers.SentenceTransformer")
    def test_load_model(self, mock_sentence_transformer, mock_huggingface_embedding, mock_path_join):
        self.index_generator.load_local = True
        self.index_generator.load_model()
        mock_huggingface_embedding.assert_called_once_with(model_name=Path('../../../../../../LLM/pretrained_models/all-MiniLM-L12-v2'))

        # Blocked by Zscalar
        # self.index_generator.load_local = False
        # self.index_generator.load_model()
        # mock_sentence_transformer.assert_called_once_with("all-MiniLM-L12-v2")


    @patch("llama_index.core.node_parser.SemanticSplitterNodeParser.get_nodes_from_documents", return_value=["node1", "node2"])
    @patch("llama_index.core.StorageContext.from_defaults")
    @patch("llama_index.core.VectorStoreIndex.__init__", return_value=None)
    @patch("llama_index.core.VectorStoreIndex.storage_context", new_callable=MagicMock)
    def test_create_index(self, mock_vector_store_index_storage_context, mock_vector_store_index_init, mock_storage_context_from_defaults, mock_get_nodes):
        mock_storage_context_instance = MagicMock()
        mock_storage_context_from_defaults.return_value = mock_storage_context_instance

        # Mocking Node objects
        mock_node = TextNode(text="Test node", metadata={})
        mock_get_nodes.return_value = [mock_node, mock_node]

        documents = [Document(text="Test document")]
        index = self.index_generator.create_index(documents)

        mock_get_nodes.assert_called_once_with(documents, show_progress=False)
        mock_storage_context_instance.docstore.add_documents.assert_called_once_with([mock_node, mock_node])
        mock_vector_store_index_init.assert_called_once_with([mock_node, mock_node], storage_context=mock_storage_context_instance, show_progress=False)
        self.assertIsInstance(index, MagicMock)  # Ensure index is created (mocked instance in this case)


    @patch("llama_index.core.VectorStoreIndex.storage_context")
    def test_save_index(self, mock_storage_context):
        index = MagicMock()
        self.index_generator.save_index(index)
        index.storage_context.persist.assert_called_once_with()

    @patch("builtins.print")
    def test_generate_index_dry_run(self, mock_print):
        self.index_generator.dry_run = True
        self.index_generator.generate_index()
        mock_print.assert_called_with("Dry run: Skipping actual index generation.")

if __name__ == "__main__":
    unittest.main()
