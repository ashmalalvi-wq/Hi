import socket

ESP32_IP = "192.168.1.223"
ESP32_PORT = 4210

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

def send_command(cmd):
    sock.sendto(cmd.encode(), (ESP32_IP, ESP32_PORT))

inside = True
while inside == True:
    input_str = input("Enter command (F, B, L, R): ")
    if input_str == "exit":
        inside = False
    else:
        send_command(input_str)
        print(f"Sent: {input_str}")