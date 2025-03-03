from flask import Flask, request, render_template_string, redirect, url_for

app = Flask(__name__)

# Global block list stored in Python
BLOCK_LIST = []

# File name to store blocked URLs (must match what the C code reads)
BLOCK_LIST_FILE = "block_list.txt"

def save_block_list():
    with open(BLOCK_LIST_FILE, "w") as f:
        for url in BLOCK_LIST:
            f.write(url + "\n")

# HTML template for the management console.
HTML_TEMPLATE = """
<html>
  <head>
    <title>Proxy Management Console</title>
  </head>
  <body>
    <h1>Proxy Management Console</h1>
    {% if message %}
      <p><strong>{{ message }}</strong></p>
    {% endif %}
    <ul>
      <li>
        <form action="{{ url_for('block') }}" method="post">
          Block URL: <input type="text" name="url">
          <input type="submit" value="Block">
        </form>
      </li>
      <li>
        <form action="{{ url_for('unblock') }}" method="post">
          Unblock URL: <input type="text" name="url">
          <input type="submit" value="Unblock">
        </form>
      </li>
    </ul>
    {% if block_list %}
      <h2>Blocked URLs</h2>
      <ul>
        {% for url in block_list %}
          <li>{{ url }}</li>
        {% endfor %}
      </ul>
    {% endif %}
  </body>
</html>
"""

@app.route("/")
def index():
    return render_template_string(HTML_TEMPLATE, message="", block_list=BLOCK_LIST)

@app.route("/block", methods=["POST"])
def block():
    url = request.form.get("url", "").strip()
    if url:
        if url not in BLOCK_LIST:
            BLOCK_LIST.append(url)
            message = f"Blocked URL: {url}"
        else:
            message = f"URL already blocked: {url}"
        save_block_list()
    else:
        message = "No URL provided."
    return render_template_string(HTML_TEMPLATE, message=message, block_list=BLOCK_LIST)

@app.route("/unblock", methods=["POST"])
def unblock():
    url = request.form.get("url", "").strip()
    if url:
        if url in BLOCK_LIST:
            BLOCK_LIST.remove(url)
            message = f"Unblocked URL: {url}"
        else:
            message = f"URL not in block list: {url}"
        save_block_list()
    else:
        message = "No URL provided."
    return render_template_string(HTML_TEMPLATE, message=message, block_list=BLOCK_LIST)

if __name__ == "__main__":
    # Run the management console on port 3000.
    app.run(host="0.0.0.0", port=3000)
