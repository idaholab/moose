from pathlib import Path
import requests
import os
import certifi
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

# Set up GitHub credentials & variables
load_dotenv()
username = 'MOOSEbot'
repo_owner = 'MengnanLi91'
repo = 'moose'
end_point = "https://api.github.com/graphql"
discussion_arr = 1  # Number of discussions to fetch
GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")


def load_database(db_dir):
    vector_store = SimpleVectorStore.from_persist_dir(db_dir)
    storage_context = StorageContext.from_defaults(
        vector_store=vector_store, persist_dir=db_dir
    )
    index = load_index_from_storage(storage_context=storage_context)

    return index

def generate_solution(title, top_n, index, threshold):

    # Compute similarities
    retriever = VectorIndexRetriever(index=index,similarity_metric='cosine', similarity_top_k=top_n)
    retrieved_nodes = retriever.retrieve(QueryBundle(title))

    processor = SimilarityPostprocessor(similarity_cutoff=threshold)
    filtered_nodes = processor.postprocess_nodes(retrieved_nodes)

    result = []
    result.append(f"Here are some previous posts that may related to your question: \n\n")

    for index, node in enumerate(filtered_nodes):

        result.append(f"    {index + 1}. Title: {node.metadata['title']}")
        result.append(f"    URL: [{node.metadata['url']}]({node.metadata['url']})")
        result.append(f"    Similarity: {node.score}\n")


    return "\n".join(result)

def query_response(top_n, index, threshold):
        # GraphQL query to fetch discussions
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

    # GraphQL mutation to add a comment
    mutation = '''
    mutation($discussionId: ID!, $body: String!) {
    addDiscussionComment(input: {discussionId: $discussionId, body: $body}) {
        comment {
        id
        }
    }
    }
    '''

    # Variables for the GraphQL queries/mutations
    variables = {
    'owner': repo_owner,
    'repo': repo,
    'first': discussion_arr
    }

    # Construct the request headers
    headers = {"Authorization": "bearer {}".format(GITHUB_TOKEN)}


    # Send the GraphQL request to fetch discussions
    response = requests.post(end_point, json={'query': query, 'variables': variables}, headers=headers, verify=certifi.where())

    # Check if the request was successful
    if response.status_code == 200:
        data = response.json()
        # Extract discussions from the response
        discussions = data['data']['repository']['discussions']['nodes']

        # Loop through each discussion
        for discussion in discussions:
            title = discussion['title']
            author = discussion['author']['login']
            comments = discussion['comments']['nodes']

            # Check if a response has already been provided
            if comments:
                existing_comment_author = comments[0]['author']['login']

                # If a response has been provided, skip to the next discussion
                if existing_comment_author != username:
                    continue

            # Generate a concise solution
            concise_solution = generate_solution(title, top_n, index, threshold)

            # Construct the response body with the bot tag and warning
            response_body = f"Hey, @{author}\n\n {concise_solution} \n\nNote: This is an automated response. Please review and verify the solution. \n @{username} [BOT]"

            # Get the discussion ID
            discussion_id = discussion['id']

            # Send the GraphQL mutation to add a comment
            response = requests.post(end_point, json={'query': mutation, 'variables': {
                                    'discussionId': discussion_id, 'body': response_body}}, headers=headers)

            # Check if the mutation request was successful
            if response.status_code == 200:
                response_data = response.json()
                comment_id = response_data['data']['addDiscussionComment']['comment']['id']
                print(
                    f"Successfully replied to discussion: {title} (Comment ID: {comment_id})")
            else:
                print(f"Failed to add comment to discussion: {title}")
    else:
        print(f"Request failed with status code: {response.status_code}")
        print(response.text)



if __name__ == "__main__":
    # Model used for encoding posts
    # This model should be the same as the one used in build_db.py

    embed_model = HuggingFaceEmbedding(model_name="sentence-transformers/all-MiniLM-L12-v2")

    # Database directory
    db_dir = Path("database")
    # Top N most similar posts to retrieve
    top_n = 5
    # Cutoff threshold
    threshold = 0.2

    Settings.embed_model = embed_model

    #load database
    index = load_database(db_dir)

    query_response(top_n, index, threshold)



## Reference
# 1. https://github.com/dhrubasaha08/Discuss-Bot
