from pathlib import Path
import requests
import json
from dotenv import load_dotenv
import os
import certifi

end_point = "https://api.github.com/graphql"
num_discussion = 10
num_comment = 10
num_reply = 10
min_credit = 100
STATUS_SUCCESS = 200
out_dir = Path("response")


def log(begin_cursor, end_cursor, remaining, has_next_page):
    print("            From: {}".format(begin_cursor))
    print("              To: {}".format(end_cursor))
    print("Remaining credit: {}".format(remaining))
    print("        Has more: {}".format(has_next_page))
    print("-" * 79)


if __name__ == "__main__":
    load_dotenv()
    query_template = Path("query.gql.in").read_text()
    GITHUB_TOKEN = os.getenv("GITHUB_TOKEN")
    headers = {"Authorization": "bearer {}".format(GITHUB_TOKEN)}

    has_remaining_credit = True
    has_next_page = True
    end_cursor = "null"
    out_dir.mkdir(exist_ok=True)
    while has_next_page and has_remaining_credit:
        query = query_template.format(
            end_cursor,
            num_discussion,
            num_comment,
            num_reply,
        )
        response = requests.post(end_point, headers=headers, json={"query": query},verify=certifi.where())
        if response.status_code == STATUS_SUCCESS:
            result = response.json()
            begin_cursor = end_cursor
            page_info = result["data"]["repository"]["discussions"]["pageInfo"]
            has_next_page = page_info["hasNextPage"]
            end_cursor = '"' + page_info["endCursor"] + '"'
            remaining = result["data"]["rateLimit"]["remaining"]
            has_remaining_credit = remaining >= min_credit
            log(begin_cursor, end_cursor, remaining, has_next_page)
            out_file = out_dir / "{}_{}.json".format(begin_cursor, end_cursor)
            json.dump(result["data"]["repository"], out_file.open("w"), indent=2)
        else:
            print("Error: {}".format(response.status_code))
            print(response.text)
            exit()
