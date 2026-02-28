import sys
from google import genai

# --- PASTE YOUR GEMINI API KEY HERE ---
API_KEY = "AIzaSyANSJ0LW-0B76DsxpXqND8ZBADOxIpOXV4"

def main():
    if len(sys.argv) < 3:
        print("Howdy! I seem to have lost my voice.")
        return

    npc_name = sys.argv[1]
    item_name = sys.argv[2]

    client = genai.Client(api_key=API_KEY)
    
    # Using the flash model for speed

    # We ask for a short response so it fits neatly in your Raylib UI box
    prompt = (f"You are a wild west character named {npc_name}. "
              f"In one or two very short sentences, ask the player to find and bring you a {item_name}. "
              f"Speak in a heavy wild west dialect. Keep it under 25 words.")

    try:
        response = client.models.generate_content(
            model="gemini-2.5-flash",
            contents=prompt
        )
        print(response.text.strip().replace('\n', ' '))
    except Exception as e:
        print("Gosh darn it, my brain ain't working right now! (API Error)")
        print(response.text)

if __name__ == "__main__":
    main()
