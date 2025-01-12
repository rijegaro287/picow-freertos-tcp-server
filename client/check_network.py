import socket
from ipaddress import ip_network

def find_server_in_network(subnet, port):
    """
    Scan the network for a server with an open port.
    """
    print(f"Scanning {subnet} for a server on port {port}...")
    for ip in ip_network(subnet, strict=False).hosts():
        ip = str(ip)
        print(f"Checking {ip}...")
        try:
            with socket.create_connection((ip, port), timeout=0.5) as s:
                print(f"Server detected at {ip}:{port}")
                return ip
        except (socket.timeout, ConnectionRefusedError, OSError):
            pass
    print("Server not found in the subnet.")
    return None

if __name__ == "__main__":
    # Replace with your subnet and server's port
    subnet = "192.168.100.0/24"  # Adjust for your network
    port = 8080  # Replace with the server's listening port
    ip = find_server_in_network(subnet, port)
    if ip:
        print(f"Server is online at {ip}:{port}")
    else:
        print("Server is not detected.")
