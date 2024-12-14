import socket
import json
import os

def send_request(data, client):
    """Send JSON request to the server and receive a response."""
    print(data)
    client.sendall(json.dumps(data).encode()+ b"\n")  # Send data
    print('send data')

    response = b""
    print('b')
    while True:
        chunk = client.recv(2048)
        if b"<END>" in chunk:
            response += chunk.split(b'<END>')[0]
            break
        print(chunk)
        response += chunk
    print('response')
    print(response)
    print("Raw Response:", response.decode())
    return json.loads(response.decode())

def upload_file(file_path, file_name, client):
    """Upload a file to the server."""
    with open(file_path, "rb") as f:
        data = f.read()

    print("send start")
    # Send upload metadata
    send_request({
        "action": "upload",
        "file_name": file_name,
        "file_size": len(data),
    }, client)
    print("metadata sent")

    # Send raw file data
    client.sendall(data)
    print("All data sent to server")

    response = b""
    while True:
        print('a')
        chunk = client.recv(2048)
        if b"<END>" in chunk:
            response += chunk.split(b'<END>')[0]
            break
        response += chunk
        print('response')
        print(response)
        print("Raw Response:", response.decode())
        return json.loads(response.decode())
    print(response)
    print(f"File {file_name} uploaded successfully.")

def list_files(client):
    """List files available on the server."""
    response = send_request({"action": "list_files"}, client)
    print(response)
    print("Available Files:", response.get("files", []))
    return response.get("files", [])

def download_file(file_name, save_path, client):
    """Request a file download from the server."""
    # Send download request
    client.sendall(json.dumps({"action": "download", "file_name": file_name}).encode())

    # Receive file data
    with open(save_path, "wb") as f:
        while True:
            chunk = client.recv(2048)
            if b"<END>" in chunk:
                f.write(chunk.split(b'<END>')[0])
                break
            f.write(chunk)

    print(f"File {file_name} downloaded and saved to {save_path}")

# Test all functions
if __name__ == "__main__":
    # Create a persistent client socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        client.connect(("127.0.0.1", 12345))

        # Test file upload
        test_file = "test_image.webp"
        upload_file(test_file, "server_test_image.webp", client)

        # Test file listing
        list_files(client)

        # Test file download
        download_file("server_test_image.webp", "downloaded_test_image.webp", client)
