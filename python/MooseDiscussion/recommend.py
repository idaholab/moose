from sentence_transformers import SentenceTransformer, util
from pathlib import Path
import pandas as pd
import numpy as np
import requests
import os
import certifi
from dotenv import load_dotenv

# Set up GitHub credentials & variables
load_dotenv()
username = 'MOOSEbot'
repo_owner = 'MengnanLi91'
repo = 'moose-gh-mining'
end_point = "https://api.github.com/graphql"
discussion_arr = 1  # Number of discussions to fetch
GITHUB_TOKEN = os.getenv("API_KEY")

def generate_solution(title, top_n, meta):

    # Compute similarities
    question_encoding = model.encode(title)
    similarities = util.pytorch_cos_sim(question_encoding, encodings)

    result = []
    result.append(f"Here are some previous posts that may related to your question: {title} \n\n")

    # Ensure similarities is a 1-dimensional tensor
    similarities = similarities.squeeze()

    sorted_idx = similarities.argsort(descending=True)
    top_n_idx = sorted_idx[:top_n]

    for j, idx in enumerate(top_n_idx):
        result.append(f"    {j + 1}. Title: {meta['title'].iloc[idx.item()]}")
        result.append(f"        URL: {meta['url'].iloc[idx.item()]}")
        result.append(f"        Similarity: {similarities[idx].item():.4f}")

    return "\n".join(result)

def query_response(model, top_n, meta, encodings):
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
            concise_solution = generate_solution(title, top_n, meta)

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
    model = SentenceTransformer("all-MiniLM-L6-v2")

    # Database directory
    db_dir = Path("db")
    # Top N most similar posts to retrieve
    top_n = 5

    # Read database
    meta = pd.read_csv(db_dir / "meta.csv")
    encodings = np.load(db_dir / "encoding.npy")

    #print(generate_solution("How do I make my code more efficient?", top_n, meta))

    query_response(model, top_n, meta, encodings)



## Reference
# 1. https://github.com/dhrubasaha08/Discuss-Bot
# 2. https://github.com/hugary1995/moose-gh-mining
