import wave
import socket
import time

SEND_NEXT = '1'

wave_file = wave.open('./test_data/sine_wave_1khz.wav', 'rb')
n_channels = wave_file.getnchannels()
n_samples = wave_file.getnframes()
sample_width = wave_file.getsampwidth()
sample_rate = wave_file.getframerate()

print(f'Number of samples: {n_samples}')
print(f'Sample width: {sample_width} bytes')
print(f'Sample rate: {sample_rate}')

ip_addr = "192.168.100.70"
port = 4242

buffer_size = 1024

total_retries = 0
sent_bytes = 0

start = time.time()
for i in range(0, n_samples, buffer_size // sample_width):
    print(f'-- Bytes sent: {sent_bytes}')
    print(f'-- Total retries: {total_retries}')

    frames = wave_file.readframes(buffer_size // sample_width)

    response = ''
    retries = 0
    while response != SEND_NEXT:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect((ip_addr, port))

        client_socket.send(frames)
        client_socket.settimeout(3)
        
        response = client_socket.recv(buffer_size).decode()
        
        print(f'-- Sending Next: {True if response == SEND_NEXT else False}')

        if not response:
            print('-- Connection closed by server')
            break

        if response == SEND_NEXT:
            sent_bytes += buffer_size
        else:
            retries += 1

        client_socket.settimeout(None)
        client_socket.close()

    total_retries += retries

print(f'Elapsed time: {time.time() - start} seconds')

wave_file.close()
