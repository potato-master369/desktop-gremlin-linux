# CLI tool to clean up kurt's garbage
# Some assetpacks are misconfigured by frame count,
# and if that's the outro, we segfault and die
# which is bad cos we have an audio lock
import sys
import os
from PIL import Image
from dataclasses import dataclass

@dataclass
class config:
    width: int = 0
    height: int = 0
    scale: float = 0.0

@dataclass
class config_out:
    runup: int = 0
    rundown: int = 0
    runleft: int = 0
    runright: int = 0
    upleft: int = 0
    upright: int = 0
    downleft: int = 0
    downright: int = 0
    emote1: int = 0
    emote2: int = 0
    emote3: int = 0
    emote4: int = 0
    walkdown: int = 0
    walkleft: int = 0
    walkright: int = 0
    walkup: int = 0
    grab: int = 0
    hover: int = 0
    idle: int = 0
    intro: int = 0
    click: int = 0
    outro: int = 0
    pat: int = 0
    runidle: int = 0
    sleep: int = 0
    width: int = 0
    height: int = 0
    column: int = 0
    scale: float = 0.0

asset_conf = config()
asset_conf_out = config_out()

def cache_config(config_path: str) -> None:
    print(" [ fixup ] Reading config.txt...")
    with open(config_path, 'r') as f:
        for line in f:
            clean_line = line.strip()
            if not clean_line or clean_line.startswith('//'):
                continue
            if '=' in clean_line:
                raw_key, raw_value = clean_line.split('=', 1)
                key = raw_key.strip()
                value = raw_value.strip()
                print(f" [ fixup ] key {key}, value {value}")
                match key:
                    case "WIDTH":
                        asset_conf.width = int(value)
                        print(f" [ fixup ] Setting width to {int(value)}")
                    case "HEIGHT":
                        asset_conf.height = int(value)
                        print(f" [ fixup ] Setting height to {int(value)}")
                    case "SCALE":
                        asset_conf.scale = float(value)
                        print(f" [ fixup ] Setting scale to {float(value)}")

def check_img(path: str) -> int:
    print(f" [ fixup ] Checking image at {path}...")
    with Image.open(path).convert('RGBA') as img:
        if asset_conf_out.width > 0:
            asset_conf_out.column = img.width / asset_conf_out.width
            
        fw, fh = asset_conf_out.width, asset_conf_out.height
        cols = img.width // fw
        rows = img.height // fh
        
        last_valid_index = 0
        current_index = 0
        
        for y in range(0, img.height, fh):
            for x in range(0, img.width, fw):
                current_index += 1
                tile = img.crop((x, y, x + fw, y + fh))
                extrema = tile.getextrema()
                if extrema and extrema[3][1] > 10:
                    last_valid_index = current_index
                    
        return last_valid_index

def write_conf(path: str) -> int:
    with open(path, 'w') as f:
        f.write(f"""//Run
RUNUP={asset_conf_out.runup}
RUNDOWN={asset_conf_out.rundown}
RUNLEFT={asset_conf_out.runleft}
RUNRIGHT={asset_conf_out.runright}

//Run Diagonal
UPLEFT={asset_conf_out.upleft}
UPRIGHT={asset_conf_out.upright}
DOWNLEFT={asset_conf_out.downleft}
DOWNRIGHT={asset_conf_out.downright}

//Emotes
EMOTE1={asset_conf_out.emote1}
EMOTE2={asset_conf_out.emote2}
EMOTE3={asset_conf_out.emote3}
EMOTE4={asset_conf_out.emote4}

// Walk
WALKDOWN={asset_conf_out.walkdown}
WALKLEFT={asset_conf_out.walkleft}
WALKRIGHT={asset_conf_out.walkright}
WALKUP={asset_conf_out.walkup}

//Actions
GRAB={asset_conf_out.grab}
HOVER={asset_conf_out.hover}
IDLE={asset_conf_out.idle}
INTRO={asset_conf_out.intro}
OUTRO={asset_conf_out.outro}
PAT={asset_conf_out.pat}
RUNIDLE={asset_conf_out.runidle}
SLEEP={asset_conf_out.sleep}
CLICK={asset_conf_out.click}

//SpriteSheet
WIDTH={asset_conf_out.width}
HEIGHT={asset_conf_out.height}
COLUMN={asset_conf_out.column}
SCALE={asset_conf_out.scale}
""")

if __name__ == "__main__":
    print("desktop-gremlin-linux v4.x.x fixup")
    assetpack = ""
    asset_width = -1
    asset_height = -1
    # Check for our arguments
    if len(sys.argv) < 2:
        print(f"Usage: python3 {sys.argv[0]} <assetpack> [OPTIONS]")
        sys.exit(1)
    else:
        print(" [ fixup ] Storing argv info...")
        if (len(sys.argv) == 2): 
            if sys.argv[1] == "--help":
                # print help
                print("""
                fixup help:
                python3 fixup.py <assetpack> [OPTIONS]
                assetpack: folder from which to get assets (contains config.txt;
                           if its named with caps rename it config.txt
                           (lower-case) now)
                OPTIONS:
                --width N    Sets the width per-sprite to N
                --height N   Sets the height per-sprite to N
                --help       Shows this help message.
                """)
                sys.exit(0)
            else:
                assetpack = sys.argv[1]
        else:
            if sys.argv[1] == "--help":
                # print help
                print("""
                fixup help:
                python3 fixup.py <assetpack> [OPTIONS]
                assetpack: folder from which to get assets (contains config.txt;
                           if its named with caps rename it config.txt
                           (lower-case) now)
                OPTIONS:
                --width N    Sets the width per-sprite to N
                --height N   Sets the height per-sprite to N
                --help       Shows this help message.
                """)
                sys.exit(0)
            else:
                assetpack = sys.argv[1]
            for i in range(2, len(sys.argv)):
                if sys.argv[i] == "--width":
                    if i + 1 < len(sys.argv):
                        asset_width = int(sys.argv[i+1])
                    else:
                        print(" [ fixup ] \033[31mFATAL: --width N requires an argument to set the width. Please provide.\033[0m")
                        sys.exit(1)
                if sys.argv[i] == "--height":
                    if i + 1 < len(sys.argv):
                        asset_height = int(sys.argv[i+1])
                    else:
                        print(" [ fixup ] \033[31mFATAL: --height N requires an argument to set the height. Please provide.\033[0m")
                        sys.exit(1)
                if sys.argv[i] == "--help":
                    print("""
                    fixup help:
                    python3 fixup.py <assetpack> [OPTIONS]
                    assetpack: folder from which to get assets (contains config.txt)
                               if its named with caps rename it config.txt
                               (lower-case) now)
                    OPTIONS:
                    --width N    Sets the width per-sprite to N
                    --height N   Sets the height per-sprite to N
                    --help       Shows this help message.
                    """)
                    sys.exit(0)
    if os.path.isdir(assetpack) and os.path.isfile(os.path.join(assetpack, "config.txt")):
        print(" [ fixup ] Assetpack found! Continuing...")
        # cache config
        cache_config(os.path.join(assetpack, "config.txt"))
        # get the width
        if asset_width == -1 or asset_height == -1:
            is_valid = input(" [ fixup ] Does the config.txt have a correct WIDTH and HEIGHT? [Y/n] ")
            if is_valid.lower() == "y" or len(is_valid) == 0:
                print(" [ fixup ] Reading WIDTH and HEIGHT from config...")
                # read it from config
                asset_width = asset_conf.width
                asset_height = asset_conf.height
            else:
                if asset_width == -1:
                    asset_width = int(input(" [ fixup ] Input a positive integer for WIDTH: "))
                if asset_height == -1:
                    asset_height = int(input(" [ fixup ] Input a positive integer for HEIGHT: "))
                if asset_height <= 0 or asset_width <= 0:
                    print(" [ fixup ] \033[31mFATAL: invalid values provided for WIDTH and/or HEIGHT. Must be positive integers greater than 0.\033[0m")
                    sys.exit(1)
         # process
        asset_scale = 0.0

         # final check for WIDTH and HEIGHT
        if asset_width <= 0 or asset_height <= 0:
             print(" [ fixup ] \033[31mFATAL: Invalid values from config for WIDTH and/or HEIGHT. Is config.txt valid?\033[0m")
             sys.exit(1)

        is_valid = input(" [ fixup ] Shall we use config.txt's value for scale? [Y/n] ")
        if is_valid and is_valid.lower() == "n":
            asset_scale = float(input(" [ fixup] Enter a float value for SCALE: "))
        else:
            asset_scale = asset_conf.scale
        asset_conf_out.scale = asset_scale
        asset_conf_out.height = asset_height
        asset_conf_out.width = asset_width
        # Process each asset image and update config_out
        asset_paths = {
            "runup": "Run/runUp.png",
            "rundown": "Run/runDown.png",
            "runleft": "Run/runLeft.png",
            "runright": "Run/runRight.png",
            "upleft": "Run/upLeft.png",
            "upright": "Run/upRight.png",
            "downleft": "Run/downLeft.png",
            "downright": "Run/downRight.png",
            "emote1": "Emotes/emote1.png",
            "emote2": "Emotes/emote2.png",
            "emote3": "Emotes/emote3.png",
            "emote4": "Emotes/emote4.png",
            "walkdown": "Walk/walkDown.png",
            "walkleft": "Walk/walkLeft.png",
            "walkright": "Walk/walkRight.png",
            "walkup": "Walk/walkUp.png",
            "grab": "Actions/grab.png",
            "hover": "Actions/hover.png",
            "idle": "Actions/idle.png",
            "intro": "Actions/intro.png",
            "click": "Actions/click.png",
            "outro": "Actions/outro.png",
            "runidle": "Actions/runIdle.png",
            "sleep": "Actions/sleep.png",
        }

        for key, relative_path in asset_paths.items():
            full_path = os.path.join(assetpack, relative_path)
            if os.path.exists(full_path):
                frame_count = check_img(full_path)
                setattr(asset_conf_out, key, frame_count)
        print(" [ fixup ] Backing up config.txt to config.old...")
        os.rename(os.path.join(assetpack, "config.txt"), os.path.join(assetpack, "config.old"))
        print(" [ fixup ] Writing new config...")
        write_conf(os.path.join(assetpack, "config.txt"))
        print(f" [ fixup ] \033[32mDone! Saved to {assetpack}.\033[0m")
    else:
        print(" [ fixup ] \033[31mFATAL: is not an assetpack. Folder does not exist or could not find config.txt\033[0m")
        sys.exit(1)

