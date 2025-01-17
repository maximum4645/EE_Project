from flask import Flask, request, render_template, send_file, jsonify, Response
from io import BytesIO
import time

app = Flask(__name__)

# Store the latest image and timestamp in memory (initially None)
latest_image = None
latest_timestamp = None

@app.route('/', methods=['GET'])
def display_image():
    if latest_image:
        return render_template('index.html', timestamp=latest_timestamp)
    else:
        return "No image received yet"

@app.route('/image')
def get_image():
    if latest_image:
        return send_file(BytesIO(latest_image), mimetype='image/jpeg')
    else:
        return "No image available"

@app.route('/', methods=['POST'])
def receive_image():
    global latest_image, latest_timestamp
    image_data = request.data
    latest_image = image_data
    latest_timestamp = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())  # Save current timestamp

    print(f"Image received at {latest_timestamp}")
    return "Image received", 200

# SSE to notify client when a new image is available
@app.route('/events')
def events():
    def event_stream():
        global latest_timestamp
        while True:
            time.sleep(0.1)
            if latest_timestamp:
                yield f'data: {latest_timestamp}\n\n'
    return Response(event_stream(), content_type='text/event-stream')

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
