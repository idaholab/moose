from pathlib import Path
import requests
import os
import certifi
import argparse
from dotenv import load_dotenv
from llama_index.core.vector_stores import SimpleVectorStore
from llama_index.embeddings.huggingface import HuggingFaceEmbedding
from llama_index.core import (
    load_index_from_storage,
    QueryBundle,
    StorageContext,
    Settings,
)
from llama_index.core.retrievers import VectorIndexRetriever
from llama_index.core.postprocessor import SimilarityPostprocessor
from typing import List


class GitHubBot:
    def __init__(self, db_dir: Path, top_n: int, threshold: float, model_name: str, dry_run: bool) -> None:
        self.username = 'MOOSEbot'
        self.repo_owner = 'MengnanLi91'
        self.repo = 'moose'
        self.end_point = "https://api.github.com/graphql"
        self.discussion_arr = 1  # Number of discussions to fetch
        load_dotenv()
        self.github_token = os.getenv("GITHUB_TOKEN")
        self.top_n = top_n
        self.threshold = threshold
        self.dry_run = dry_run
        self.index = self.load_database(db_dir)

        # Load the embedding model
        try:
            self.embed_model = HuggingFaceEmbedding(model_name=model_name)
        except Exception as e:
            print(f"Failed to load the model from HuggingFace: {e}")
            print("Please provide a local path to the model as model_name")
            self.embed_model = HuggingFaceEmbedding(model_name=model_name)

        Settings.embed_model = self.embed_model

    def load_database(self, db_dir: Path) -> SimpleVectorStore:
        vector_store = SimpleVectorStore.from_persist_dir(db_dir)
        storage_context = StorageContext.from_defaults(
            vector_store=vector_store, persist_dir=db_dir
        )
        index = load_index_from_storage(storage_context=storage_context)
        return index

    def generate_solution(self, title: str, top_n: int, index: SimpleVectorStore, threshold: float) -> str:
        retriever = VectorIndexRetriever(index=index, similarity_metric='cosine', similarity_top_k=top_n)
        retrieved_nodes = retriever.retrieve(QueryBundle(title))

        processor = SimilarityPostprocessor(similarity_cutoff=threshold)
        filtered_nodes = processor.postprocess_nodes(retrieved_nodes)

        result: List[str] = []
        result.append(f"Here are some previous posts that may relate to your question: \n\n")

        for idx, node in enumerate(filtered_nodes):
            result.append(f"    {idx + 1}. Title: {node.metadata['title']}")
            result.append(f"    URL: [{node.metadata['url']}]({node.metadata['url']})")
            result.append(f"    Similarity: {node.score}\n")

        return "\n".join(result)

    def query_response(self) -> None:
        query = '''
        query($owner: String!, $repo: String!, $first: Int!) {
        repository(owner: $owner, name: $repo) {
            discussions(first: $first) {
            totalCount
            pageInfo {
                hasNextPage
                endCursor
            }
            nodes {
                id
                title
                body
                author {
                login
                }
                comments(first: 1) {
                nodes {
                    author {
                    login
                    }
                    body
                }
                }
            }
            }
        }
        }
        '''

        mutation = '''
        mutation($discussionId: ID!, $body: String!) {
        addDiscussionComment(input: {discussionId: $discussionId, body: $body}) {
            comment {
            id
            }
        }
        }
        '''

        variables = {
            'owner': self.repo_owner,
            'repo': self.repo,
            'first': self.discussion_arr
        }

        headers = {"Authorization": f"bearer {self.github_token}"}

        response = requests.post(self.end_point, json={'query': query, 'variables': variables}, headers=headers, verify=certifi.where())

        if response.status_code == 200:
            data = response.json()
            discussions = data['data']['repository']['discussions']['nodes']

            for discussion in discussions:
                title = discussion['title']
                author = discussion['author']['login']
                comments = discussion['comments']['nodes']

                if comments and comments[0]['author']['login'] != self.username:
                    continue

                concise_solution = self.generate_solution(title, self.top_n, self.index, self.threshold)
                response_body = f"Hey, @{author}\n\n {concise_solution} \n\nNote: This is an automated response. Please review and verify the solution. \n @{self.username} [BOT]"

                discussion_id = discussion['id']

                if not self.dry_run:
                    response = requests.post(self.end_point, json={'query': mutation, 'variables': {'discussionId': discussion_id, 'body': response_body}}, headers=headers)

                    if response.status_code == 200:
                        response_data = response.json()
                        comment_id = response_data['data']['addDiscussionComment']['comment']['id']
                        print(f"Successfully replied to discussion: {title} (Comment ID: {comment_id})")
                    else:
                        print(f"Failed to add comment to discussion: {title}")
                else:
                    print(f"Dry run mode: Would have replied to discussion: {title} with the following body:\n{response_body}")
        else:
            print(f"Request failed with status code: {response.status_code}")
            print(response.text)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='GitHub Bot for replying to discussions.')
    parser.add_argument('--db_dir', type=Path, default=Path("database"), help='Path to the database directory.')
    parser.add_argument('--top_n', type=int, default=5, help='Top N most similar posts to retrieve.')
    parser.add_argument('--threshold', type=float, default=0.2, help='Cutoff threshold for similarity.')
    parser.add_argument('--model_name', type=str, default="sentence-transformers/all-MiniLM-L12-v2", help='Model name for HuggingFace embedding.')
    parser.add_argument('--dry_run', action='store_true', help='Run the bot in dry run mode without posting comments.')

    args = parser.parse_args()

    bot = GitHubBot(db_dir=args.db_dir, top_n=args.top_n, threshold=args.threshold, model_name=args.model_name, dry_run=args.dry_run)
    bot.query_response()


## Reference
# 1. https://github.com/dhrubasaha08/Discuss-Bot
