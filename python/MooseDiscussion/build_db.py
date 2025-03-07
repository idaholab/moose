import pandas as pd
import json
from sentence_transformers import SentenceTransformer
from pathlib import Path
import numpy as np
import certifi

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


if __name__ == "__main__":
    # Directory for storing JSON responses fetched from GitHub
    response_dir = Path("response")
    # Database directory
    db_dir = Path("db")
    # Model used for encoding posts
    model = SentenceTransformer("all-MiniLM-L6-v2")
    # Batch size
    batch_size = 128

    titles = []
    urls = []
    contents = []
    encodings = []

    for file in response_dir.glob("*.json"):
        print("Processing", file)
        page = json.loads(file.read_text())
        posts = page["discussions"]["edges"]
        for post in posts:
            title = post["node"]["title"]
            url = post["node"]["url"]
            content = get_post_content(post["node"])

            titles.append(title)
            urls.append(url)
            contents.append(content)

            if len(contents) == batch_size:
                print("-" * 79)
                print("Encoding batch")
                encodings.append(encode_content(model, contents))
                contents = []
                print("Done")
                print("-" * 79)
    if len(contents) > 0:
        print("-" * 79)
        print("Encoding batch")
        encodings.append(encode_content(model, contents))
        contents = []
        print("Done")
        print("-" * 79)

    # Save database
    db_dir.mkdir(exist_ok=True)
    df = pd.DataFrame({"title": titles, "url": urls})
    df.to_csv(db_dir / "meta.csv")
    encoding = np.concatenate(encodings, axis=0)
    np.save(db_dir / "encoding.npy", encoding)
