import socket

HOST, PORT = '127.0.0.1', 5000
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen()

    print(f"Server listening on {HOST}:{PORT}...")
    while True:
        conn, addr = s.accept()            # Loop back to accept new clients :contentReference[oaicite:2]{index=2}
        with conn:
            print('Connected by', addr)
            while True:
                data = conn.recv(1024)
                if not data:
                    break
                val = float(data.decode())
                conn.sendall(str(val * 2).encode())
