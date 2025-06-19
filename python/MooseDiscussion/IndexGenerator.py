import json
import argparse
from pathlib import Path
import os
from typing import List, Optional, Union
from llama_index.core import SimpleDirectoryReader, Document
from llama_index.core.readers.base import BaseReader
from llama_index.core.node_parser import SemanticSplitterNodeParser
from llama_index.embeddings.huggingface import HuggingFaceEmbedding
from llama_index.core import VectorStoreIndex, StorageContext, Settings
from llama_index.core.storage.docstore import SimpleDocumentStore
from llama_index.core.storage.index_store import SimpleIndexStore
from llama_index.core.vector_stores import SimpleVectorStore
from sentence_transformers import SentenceTransformer


class IndexGenerator:
    def __init__(
        self,
        load_local: bool,
        model_path: str,
        model_name: str,
        show_progress: bool,
        rawdata: str,
        database: str,
        dry_run: bool,
    ):
        self.load_local = load_local
        self.model_path = model_path
        self.model_name = model_name
        self.show_progress = show_progress
        self.database = Path(database)
        self.dry_run = dry_run
        self.rawdata_dir = Path(rawdata)
        self.embed_model = self.load_model()

        Settings.embed_model = self.embed_model

    def get_post_content(self, post: dict) -> str:
        content = ""
        content += post["title"] + "\n"
        if "bodyText" in post:
            content += post["bodyText"] + "\n"
        if "comments" in post:
            for comment in post["comments"]["edges"]:
                content += self.get_comment_content(comment["node"])
        return content

    def get_comment_content(self, comment: dict) -> str:
        content = ""
        if "bodyText" in comment:
            content += comment["bodyText"] + "\n"
        if "replies" in comment:
            for reply in comment["replies"]["edges"]:
                content += self.get_comment_content(reply["node"])
        return content

    class MyFileReader(BaseReader):
        def __init__(self, get_post_content):
            self.get_post_content = get_post_content

        def load_data(
            self, file: str, extra_info: Optional[dict] = None
        ) -> List[Document]:
            with open(file, "r") as f:
                # page = json.loads(file.read_text())
                page = json.load(f)
                posts = page["discussions"]["edges"]
                for post in posts:
                    title = post["node"]["title"]
                    url = post["node"]["url"]
                    content = self.get_post_content(post["node"])
                return [Document(text=content, metadata={"title": title, "url": url})]

    def load_model(self) -> HuggingFaceEmbedding:
        if self.load_local:
            model_path_full = os.path.join(self.model_path, self.model_name)
            print(f"Loading local model from {model_path_full}")
            return HuggingFaceEmbedding(model_name=model_path_full)
        else:
            print(
                f"Attempting to download model '{self.model_name}' from Huggingface..."
            )
            return HuggingFaceEmbedding(
                model_name=f"sentence-transformers/{self.model_name}"
            )

    def read_documents(self) -> List[Document]:
        reader = SimpleDirectoryReader(
            input_dir=self.rawdata_dir,
            file_extractor={".json": self.MyFileReader(self.get_post_content)},
        )
        documents = reader.load_data()
        print(f"Loaded {len(documents)} docs")
        return documents

    def create_index(self, documents: List[Document]) -> VectorStoreIndex:
        splitter = SemanticSplitterNodeParser(
            embed_model=self.embed_model,
            buffer_size=50,
            breakpoint_percentile_threshold=95,
        )

        print("Generate Llama Index nodes from documentation.")
        nodes = splitter.get_nodes_from_documents(
            documents, show_progress=self.show_progress
        )

        storage_context = StorageContext.from_defaults(
            docstore=SimpleDocumentStore(),
            vector_store=SimpleVectorStore(),
            index_store=SimpleIndexStore(),
        )

        storage_context.docstore.add_documents(nodes)

        print("Generate embeddings.")
        index = VectorStoreIndex(
            nodes, storage_context=storage_context, show_progress=self.show_progress
        )
        return index

    def save_index(self, index: VectorStoreIndex):
        index.storage_context.persist(persist_dir=self.database)

    def generate_index(self):
        if self.dry_run:
            print("Dry run: Skipping actual index generation.")
            return

        documents = self.read_documents()
        index = self.create_index(documents)
        print(f"Save index to: {self.database}")
        self.save_index(index)
        print("Successfully generated the index database from documentation!")


def main():
    parser = argparse.ArgumentParser(
        description="Choose embedding model and load method."
    )
    parser.add_argument(
        "--load_local", action="store_true", help="Load embedding model locally."
    )
    parser.add_argument(
        "--show_progress", action="store_true", help="Show embedding progress."
    )
    # The transformer model needed to be downloaded locally due to INL block huggingface online mode. While running on Github workflow, it can successfully pull the model from Huggingface website
    parser.add_argument(
        "--model_path",
        type=str,
        default=Path("../../../../../../LLM/pretrained_models/"),
        help="Path to the local model.",
    )
    parser.add_argument(
        "--model_name",
        type=str,
        default="all-MiniLM-L12-v2",
        help="Model name for SentenceTransformer.",
    )
    parser.add_argument(
        "--rawdata",
        type=str,
        default="response",
        help="Path to store the index database.",
    )
    parser.add_argument(
        "--database",
        type=str,
        default="database",
        help="Path to store the index database.",
    )
    parser.add_argument(
        "--dry_run",
        action="store_true",
        help="Perform a dry run without generating the index.",
    )

    args = parser.parse_args()

    index_generator = IndexGenerator(
        load_local=args.load_local,
        model_path=args.model_path,
        model_name=args.model_name,
        show_progress=args.show_progress,
        rawdata=args.rawdata,
        database=args.database,
        dry_run=args.dry_run,
    )

    index_generator.generate_index()


if __name__ == "__main__":
    main()
