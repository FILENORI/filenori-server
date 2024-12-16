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

def download_file(file_id, save_path, client):
    """Request a file download from the server."""
    # Send download request
    client.sendall(json.dumps({"action": "download", "file_id": file_id}).encode())

    # Receive file data
    with open(save_path, "wb") as f:
        while True:
            chunk = client.recv(2048)
            if b"<END>" in chunk:
                f.write(chunk.split(b'<END>')[0])
                break
            f.write(chunk)

    print(f"File {file_id} downloaded and saved to {save_path}")

def find_file(file_id, client):
    """Find which peers have the specified file."""
    print(f"Finding file ID: {file_id}")
    response = send_request({"action": "find_file", "file_id": file_id}, client)
    print(f"Peers with file ID {file_id}: {response.get('peers', [])}")
    return response.get("peers", [])


def list_peers(client):
    """List all connected peers."""
    response = send_request({"action": "list_peers"}, client)
    print("Connected Peers:", response.get("peers", []))
    return response.get("peers", [])

def send_heartbeat(peer_ip, file_ids, client):
    """Send a heartbeat message to the server."""
    print(f"Sending heartbeat from peer {peer_ip} with files {file_ids}")
    response = send_request({
        "action": "heartbeat",
        "peer_ip": peer_ip,
        "files": file_ids
    }, client)
    print("Heartbeat response:", response)
    return response

# Test all functions
if __name__ == "__main__":
    # Create a persistent client socket
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        client.connect(("127.0.0.1", 12345))

        # Test file upload
        test_file = "test_image.webp"
        upload_file(test_file, "server_test_image.webp", client)

        # # Test file listing
        list_files(client)

        list_peers(client)

        file_id = "42b39d9b-dfd1-4eaa-8daa-75dfced71cf0"
        find_file(file_id, client)

        send_heartbeat("127.0.0.1", [file_id], client)

        # Test file download
        download_file(file_id, "downloaded_test_image.webp", client)
