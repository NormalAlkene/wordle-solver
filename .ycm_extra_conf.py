import os

DIR = os.path.abspath(os.path.dirname(__file__))

def Settings(**kwargs):
    if kwargs["language"] == "cfamily":
        return {
            "ls": {
                "compilationDatabasePath": os.path.join(DIR, "build")
            }
        }
