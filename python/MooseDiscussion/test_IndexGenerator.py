import unittest
from unittest.mock import patch, mock_open, MagicMock
import json
from llama_index.core import Document
from pathlib import Path
from IndexGenerator import IndexGenerator
import tempfile

class TestIndexGenerator(unittest.TestCase):
    def setUp(self):

      self.temp_dir = tempfile.TemporaryDirectory()
      self.rawdata = Path(self.temp_dir.name) / "mock_data.json"

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


      self.load_local = True
      self.model_path = Path('../../../../../../LLM/pretrained_models/')
      self.model_name = 'all-MiniLM-L12-v2'
      self.show_progress = False
      self.database = Path("testdatabase")
      self.dry_run = False

      self.index_generator = IndexGenerator(
          load_local=self.load_local,
          model_path=self.model_path,
          model_name=self.model_name,
          show_progress=self.show_progress,
          rawdata=self.rawdata,
          database=self.database,
          dry_run=self.dry_run
      )

    def tearDown(self):
      self.temp_dir.cleanup()

    def test_get_post_content(self):
        post = {
            "title": "Test Title",
            "bodyText": "Test Body",
            "comments": {
                "edges": [
                    {"node": {"bodyText": "Test Comment"}}
                ]
            }
        }
        content = self.index_generator.get_post_content(post)
        self.assertEqual(content, "Test Title\nTest Body\nTest Comment\n")

    def test_get_comment_content(self):
        comment = {
            "bodyText": "Test Comment",
            "replies": {
                "edges": [
                    {"node": {"bodyText": "Test Reply"}}
                ]
            }
        }
        content = self.index_generator.get_comment_content(comment)
        self.assertEqual(content, "Test Comment\nTest Reply\n")

    @patch('builtins.open', new_callable=mock_open, read_data=json.dumps({
        "discussions": {
            "edges": [
                {
                    "node": {
                        "title": "Test Title",
                        "url": "http://example.com",
                        "bodyText": "Test Body",
                        "comments": {
                            "edges": []
                        }
                    }
                }
            ]
        }
    }))
    def test_my_file_reader(self, mock_file):
        reader = self.index_generator.MyFileReader(self.index_generator.get_post_content)
        documents = reader.load_data('dummy_file.json')
        self.assertEqual(len(documents), 1)
        self.assertEqual(documents[0].text, "Test Title\nTest Body")
        self.assertEqual(documents[0].metadata["title"], "Test Title")
        self.assertEqual(documents[0].metadata["url"], "http://example.com")

    @patch("builtins.open", new_callable=unittest.mock.mock_open, read_data='{"discussions": {"edges": [{"node": {"title": "Test Title", "url": "http://example.com", "bodyText": "Test body text"}}]}}')
    def test_read_documents(self, mock_open):
        file_reader = self.index_generator.MyFileReader(self.index_generator.get_post_content)
        documents = file_reader.load_data(str(self.rawdata))
        self.assertIsInstance(documents, list)
        self.assertIsInstance(documents[0], Document)
        self.assertEqual(documents[0].text, "Test Title\nTest body text")
        self.assertEqual(documents[0].metadata, {"title": "Test Title", "url": "http://example.com"})


    @patch('llama_index.embeddings.huggingface.HuggingFaceEmbedding')
    @patch('llama_index.core.node_parser.SemanticSplitterNodeParser.get_nodes_from_documents', return_value=[Document(text="Test Node")])
    @patch('llama_index.core.vector_stores.SimpleVectorStore.add_documents')
    @patch('llama_index.core.storage.index_store.SimpleIndexStore')
    @patch('llama_index.core.vector_stores.SimpleVectorStore')
    @patch('llama_index.core.storage.docstore.SimpleDocumentStore')
    @patch('llama_index.core.VectorStoreIndex')
    def test_create_index(self, mock_index, mock_simple_docstore, mock_simple_vectorstore, mock_simple_indexstore, mock_add_documents, mock_get_nodes, mock_embedding):
        documents = [Document(text="Test Document")]
        index = self.index_generator.create_index(documents)
        self.assertIsNotNone(index)


    @patch("llama_index.core.VectorStoreIndex.storage_context")
    def test_save_index(self, mock_storage_context):
        index = MagicMock()
        self.index_generator.save_index(index)
        index.storage_context.persist.assert_called_once_with(persist_dir=Path("testdatabase"))

    @patch('IndexGenerator.IndexGenerator.read_documents', return_value=[Document(text="Test Document")])
    @patch('IndexGenerator.IndexGenerator.create_index')
    @patch('IndexGenerator.IndexGenerator.save_index')
    def test_generate_index(self, mock_save_index, mock_create_index, mock_read_documents):
        self.index_generator.generate_index()
        mock_read_documents.assert_called_once()
        mock_create_index.assert_called_once()
        mock_save_index.assert_called_once()

    @patch('IndexGenerator.IndexGenerator.generate_index')
    def test_main(self, mock_generate_index):
        test_args = ['--load_local', '--show_progress', f'--model_path={self.model_path}', f'--model_name={self.model_name}', f'--rawdata={self.rawdata}', f'--database={self.database}', '--dry_run']
        with patch('sys.argv', ['IndexGenerator.py'] + test_args):
            import IndexGenerator
            IndexGenerator.main()
            mock_generate_index.assert_called_once()

if __name__ == '__main__':
    unittest.main()
