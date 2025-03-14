import argparse
from pathlib import Path
import requests
import json
from dotenv import load_dotenv
import os
import certifi
from typing import Optional, Dict, Any

class GitHubAPI:
    def __init__(self, end_point: str, num_discussion: int, num_comment: int, num_reply: int, min_credit: int, out_dir: str, dry_run: bool) -> None:
        """
        Initialize the GitHubAPI class with endpoint, query parameters, output directory, and dry run mode.

        Parameters:
        - end_point (str): The GitHub GraphQL API endpoint.
        - num_discussion (int): Number of discussions to retrieve.
        - num_comment (int): Number of comments per discussion to retrieve.
        - num_reply (int): Number of replies per comment to retrieve.
        - min_credit (int): Minimum credit required to continue making API requests.
        - out_dir (str): Directory to save the response files.
        - dry_run (bool): Whether to run the script in dry run mode.
        """
        self.end_point: str = end_point
        self.num_discussion: int = num_discussion
        self.num_comment: int = num_comment
        self.num_reply: int = num_reply
        self.min_credit: int = min_credit
        self.out_dir: Path = Path(out_dir)
        self.STATUS_SUCCESS: int = 200
        self.has_remaining_credit: bool = True
        self.has_next_page: bool = True
        self.end_cursor: str = "null"
        self.dry_run: bool = dry_run
        load_dotenv()
        self.query_template: str = Path("query.gql.in").read_text()
        self.GITHUB_TOKEN: Optional[str] = os.getenv("GITHUB_TOKEN")
        self.headers: dict = {"Authorization": f"bearer {self.GITHUB_TOKEN}"}
        self.out_dir.mkdir(exist_ok=True)

    def log(self, begin_cursor: str, end_cursor: str, remaining: int, has_next_page: bool) -> None:
        """
        Log the details of the current API request.

        Parameters:
        - begin_cursor (str): The cursor position before the current request.
        - end_cursor (str): The cursor position after the current request.
        - remaining (int): Remaining API request credits.
        - has_next_page (bool): Whether there are more pages to fetch.
        """
        print("            From: {}".format(begin_cursor))
        print("              To: {}".format(end_cursor))
        print("Remaining credit: {}".format(remaining))
        print("        Has more: {}".format(has_next_page))
        print("-" * 79)

    def fetch_data(self) -> None:
        """
        Fetch data from the GitHub GraphQL API, paginate through results, and save responses to files.
        """
        while self.has_next_page and self.has_remaining_credit:
            query: str = self.query_template.format(
                self.end_cursor,
                self.num_discussion,
                self.num_comment,
                self.num_reply,
            )
            if self.dry_run:
                print("Dry run: would execute query with cursor {}".format(self.end_cursor))
                break

            response: requests.Response = requests.post(self.end_point, headers=self.headers, json={"query": query}, verify=certifi.where())
            if response.status_code == self.STATUS_SUCCESS:
                result: dict = response.json()
                if "data" not in result:
                    print("Error: 'data' key not found in the response.")
                    print(result)
                    exit()

                begin_cursor: str = self.end_cursor
                page_info: dict = result["data"]["repository"]["discussions"]["pageInfo"]
                self.has_next_page = page_info["hasNextPage"]
                self.end_cursor = '"' + page_info["endCursor"] + '"'
                remaining: int = result["data"]["rateLimit"]["remaining"]
                self.has_remaining_credit = remaining >= self.min_credit
                self.log(begin_cursor, self.end_cursor, remaining, self.has_next_page)
                out_file: Path = self.out_dir / "{}_{}.json".format(begin_cursor, self.end_cursor)
                with out_file.open("w") as file:
                    json.dump(result["data"]["repository"], file, indent=2)
            else:
                print("Error: {}".format(response.status_code))
                print(response.text)
                exit()

        if not self.has_next_page:
            print("All discussions have been fetched.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Fetch data from GitHub API.")
    parser.add_argument('--end_point', type=str, default="https://api.github.com/graphql", help="The GitHub GraphQL API endpoint.")
    parser.add_argument('--num_discussion', type=int, default=10, help="Number of discussions to retrieve.")
    parser.add_argument('--num_comment', type=int, default=10, help="Number of comments per discussion to retrieve.")
    parser.add_argument('--num_reply', type=int, default=10, help="Number of replies per comment to retrieve.")
    parser.add_argument('--min_credit', type=int, default=100, help="Minimum credit required to continue making API requests.")
    parser.add_argument('--out_dir', type=str, default="response", help="Directory to save the response files.")
    parser.add_argument('--dry_run', action='store_true', help="Run the script in dry run mode.")

    args = parser.parse_args()

    api = GitHubAPI(
        end_point=args.end_point,
        num_discussion=args.num_discussion,
        num_comment=args.num_comment,
        num_reply=args.num_reply,
        min_credit=args.min_credit,
        out_dir=args.out_dir,
        dry_run=args.dry_run
    )
    api.fetch_data()
