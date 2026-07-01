import serial
import socket

def wait_for_client(server_socket):
    # waits for the main program to connect
    print("Waiting for C# client...")
    server_socket.settimeout(1.0)  # 1 second timeout

    while True:
        try:
            # this attempts to establish a local connection to the c# script
            # if succesful, it basically acts to forward the arduino serial to the c# script
            # this is because c# would just not take serial inputs directly
            client_socket, addr = server_socket.accept()
            print(f"Client connected from {addr}")
            server_socket.settimeout(None)  # Remove timeout
            return client_socket
        except socket.timeout:
            pass
        except KeyboardInterrupt:
            print("Interrupted while waiting for client. Exiting.")
            raise

def main():
    # defining the serial connection to the arduino (change the com port if its different for you, check in the arduino IDE)
    ser = serial.Serial("COM8", 9600, timeout=1)

    # configuring the TCP socket
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    # defining the local host socket, its just between two programs on your pc
    server.bind(('localhost', 5555))
    server.listen(1)

    client_socket = wait_for_client(server)

    try:
        while True:
            # data input from the serial
            line = ser.readline().decode(errors='ignore').strip()
            print(line) # use this for debugging
            if line:
                try:
                    # sends the serial input over the local server to the c# script
                    client_socket.sendall((line + '\n').encode())
                except (BrokenPipeError, ConnectionResetError):
                    # error handling
                    print("Client disconnected.")
                    client_socket.close()
                    client_socket = wait_for_client(server)
    except KeyboardInterrupt:
        print("closing server (manual interupt)")
    finally:
        # closing the port when it's done being used
        ser.close()
        client_socket.close()
        server.close()

# runs allat nonsense
if __name__ == "__main__":
    main()
