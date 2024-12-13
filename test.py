import socket
import json
import os

def send_request(data, host="127.0.0.1", port=12345):
    """Send JSON request to the server and receive a response."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        client.connect((host, port))
        client.sendall(json.dumps(data).encode())

        # Receive response
        response = b""
        while True:
            chunk = client.recv(2048)
            if not chunk:
                break
            response += chunk

        print("Raw Response:", response.decode())
        return json.loads(response.decode())

def upload_file(file_path, file_name, host="127.0.0.1", port=12345):
    """Upload a file to the server."""
    with open(file_path, "rb") as f:
        data = f.read()

    # Send upload request with file content
    send_request({
        "action": "upload",
        "file_name": file_name,
        "file_size": len(data),
        "content": data.decode(errors='ignore')
    }, host, port)

def list_files(host="127.0.0.1", port=12345):
    """List files available on the server."""
    response = send_request({"action": "list_files"}, host, port)
    print("Available Files:", response.get("files", []))
    return response.get("files", [])

def download_file(file_name, save_path, host="127.0.0.1", port=12345):
    """Request a file download from the server."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        client.connect((host, port))

        # Send download request
        client.sendall(json.dumps({"action": "download", "file_name": file_name}).encode())

        # Receive file data
        with open(save_path, "wb") as f:
            while True:
                chunk = client.recv(2048)
                if not chunk:
                    break
                f.write(chunk)

    print(f"File {file_name} downloaded and saved to {save_path}")

# Test all functions
if __name__ == "__main__":
    # Test upload
    test_file = "test_upload.txt"
    with open(test_file, "w") as f:
        f.write("This is a test file for upload.\n")

    upload_file(test_file, "server_test_upload.txt")

    # Test file listing
    list_files()

    # Test download
    download_file("server_test_upload.txt", "downloaded_test_upload.txt")

    # Cleanup local test file
    os.remove(test_file)
