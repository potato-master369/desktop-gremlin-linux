# Util to generate a timestamp for the recipe

import os
from datetime import datetime, timezone

if __name__ == "__main__":
    formatted_utc = datetime.now(timezone.utc).strftime("%y-%m-%d-%H-%M-%S")

    file_path = os.path.join("recipes", "timestamp")
    with open(file_path, "w", encoding="utf-8") as file:
        file.write(formatted_utc)
