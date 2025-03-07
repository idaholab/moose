import json
import argparse
from pathlib import Path
import os
from llama_index.core import SimpleDirectoryReader, Document
from llama_index.core.readers.base import BaseReader
from llama_index.core.node_parser import SemanticSplitterNodeParser
from llama_index.embeddings.huggingface import HuggingFaceEmbedding
from llama_index.core import VectorStoreIndex, StorageContext, Settings
from llama_index.core.storage.docstore import SimpleDocumentStore
from llama_index.core.storage.index_store import SimpleIndexStore
from llama_index.core.vector_stores import SimpleVectorStore
from sentence_transformers import SentenceTransformer

def get_post_content(post):
    content = ""
    # Title
    content += post["title"] + "\n"
    # Body text
    if "bodyText" in post:
        content += post["bodyText"] + "\n"
    # Comments
    if "comments" in post:
        for comment in post["comments"]["edges"]:
            content += get_comment_content(comment["node"])
    return content

def get_comment_content(comment):
    content = ""
    # Body text
    if "bodyText" in comment:
        content += comment["bodyText"] + "\n"
    # Replies
    if "replies" in comment:
        for reply in comment["replies"]["edges"]:
            content += get_comment_content(reply["node"])
    return content

def get_reply_content(reply):
    return reply["bodyText"] + "\n"

def encode_content(model, content):
    return model.encode(content)

class MyFileReader(BaseReader):
    def load_data(self, file, extra_info=None):
        with open(file, "r") as f:
            page = json.loads(file.read_text())
            posts = page["discussions"]["edges"]
            for post in posts:
                title = post["node"]["title"]
                url = post["node"]["url"]
                content = get_post_content(post["node"])
        # load_data returns a list of Document objects
        return [Document(text=content, metadata={"title": title, "url": url})]

def load_model(load_local, model_path, model_name):
    if load_local:
        model_path_full = os.path.join(model_path, model_name)
        embed_model = HuggingFaceEmbedding(model_name=model_path_full)
    else:
        embed_model = SentenceTransformer(model_name)
    return embed_model

def read_documents(response_dir):
    reader = SimpleDirectoryReader(
        input_dir=response_dir, file_extractor={".json": MyFileReader()}
    )
    documents = reader.load_data()
    print(f"Loaded {len(documents)} docs")
    return documents

def create_index(embed_model, documents, show_progress):
    splitter = SemanticSplitterNodeParser(embed_model=embed_model,
                                          buffer_size=50, breakpoint_percentile_threshold=95)

    print("Generate Llama Index nodes from documentation.")
    nodes = splitter.get_nodes_from_documents(documents, show_progress=show_progress)


    storage_context = StorageContext.from_defaults(
        docstore=SimpleDocumentStore(),
        vector_store=SimpleVectorStore(),
        index_store=SimpleIndexStore(),
    )

    storage_context.docstore.add_documents(nodes)

    print("Generate embeddings.")
    index = VectorStoreIndex(nodes, storage_context=storage_context, show_progress=show_progress)
    return index

def save_index(index, db_dir):
    index.storage_context.persist(persist_dir=db_dir)

def main():
    parser = argparse.ArgumentParser(description="Choose embedding model and load method.")
    parser.add_argument('--load_local', action='store_true', help="Load model locally.")
    parser.add_argument('--show_progress', action='store_true', help="Show embedding progress.")
    parser.add_argument('--model_path', type=str, default="<model path>", help="Path to the local model.")
    parser.add_argument('--model_name', type=str, default="all-MiniLM-L12-v2", help="Model name for SentenceTransformer.")
    parser.add_argument('--database', type=str, default="database", help="Path to store the index database.")

    args = parser.parse_args()

    response_dir = Path("response")
    db_dir = Path(args.database)

    embed_model = load_model(args.load_local, args.model_path, args.model_name)
    Settings.embed_model = embed_model


    documents = read_documents(response_dir)
    index = create_index(embed_model, documents, args.show_progress)
    print(f"Save index to: {db_dir}")
    save_index(index, db_dir)
    print("Successfully generated the index database from documentation!")

if __name__ == "__main__":
    main()

