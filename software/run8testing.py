import serial
import socket
import time

# NOTE - THIS WILL NOT RUN IF THE ARDUINO IDE SERIAL MONITOR IS RUNNING

SERIAL_PORT = 'COM5'    # Change this to match your system (can be found in device manager or the arduino software)
BAUD_RATE = 9600        # should be properly set in arduino already, but ensure this matches what the Serial.begin() value is

# Configure UDP connection to Run8
RUN8_IP = '127.0.0.1'   # should be a generic ip that doesn't need changing
RUN8_PORT = 18888       # found / set in Run8's controls menu (under usb device)
RUN8_RECIEVE_PORT = 18889
# Create UDP socket
udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
udp_read_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

udp_read_socket.bind((RUN8_IP, RUN8_RECIEVE_PORT))

parsed_data = {}
line = ""

def sendByte(ushort, value):
    data = [96, 0, int(ushort)]
    data.append(value)
    crc = data[0]
    for i in range(1, len(data)):  
        crc ^= data[i]
    data.append(crc)
    byte_data = bytes(data)
    udp_socket.sendto(byte_data, (RUN8_IP, RUN8_PORT))

udp_socket.settimeout(1.0)
udp_read_socket.setblocking(False)

last_sent = {}

# try, except for error handling
try:
    parsed_data = {}
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
    print(f"Listening on {SERIAL_PORT} at {BAUD_RATE} baud...")
    
    while True:
        parsed_data = {}
        # print("Loop tick")
        # Wait until a full line is received
        line = ser.readline().decode('utf-8').strip()

        if not line:  
            continue  # ignore empty lines

        # checks for the leading comma from arduino and removes it (otherwise the split doesn't work)
        if line.startswith(","):
            line = line[1:]

        # confirms what data is recieved
        print("Received:", line)

        # attemps to extract values, lines get skipped if it fails.  If it fails constantly, something is very wrong
        # this should at least prevent the program from crashing though
        try:
            sections = line.split(',')
            for section in sections:
                key, value = section.split(':')
                parsed_data[key] = int(value)  # Convert value to integer
        except ValueError:
            print("bad line, data skipped:", line)
            continue
        
        

        # if these values aren't sent by arduino they just end up being equal to 0, so don't worry about enabling or disabling them
        HR = parsed_data.get("HR", 0)
        BL = parsed_data.get("BL", 0)
        SD = parsed_data.get("SD", 0)
        AB = parsed_data.get("AB", 0)
        IB = parsed_data.get("IB", 0)
        RS = parsed_data.get("RS", 0)
        IL = parsed_data.get("IL", 0)
        TH = parsed_data.get("TH", 0)
        DB = parsed_data.get("DB", 0)
        AL = parsed_data.get("AL", 0)
        GF = parsed_data.get("GF", 0)
        FP = parsed_data.get("FP", 0)
        ER = parsed_data.get("ER", 0)
        GL = parsed_data.get("GL", 0)
        SL = parsed_data.get("SL", 0)
        DL = parsed_data.get("DL", 0)
        HL = parsed_data.get("HL", 0)
        RL = parsed_data.get("RL", 0)

        # the great list of checking for controls is after the sendByte function 
        # in theory, it should not need to be modified, as the code checks what the configured arudino code sends

        # the great function (ushort is the value listed in the run8 docs, value is the value being sent to the sim)
        # this can now be called for any control by passing the ushort to the function (instead of needing a whole new block of code)
        

        # the greatest if statement list of all time
        # checks if control data is recieved, then if it is recieved, it forwards the data to run8 via the sendByte() function
        
        for key, value in parsed_data.items():
            if last_sent.get(key) != value:
                # map key to ushort
                if key == "HR": sendByte(8, value)
                elif key == "BL": sendByte(2, value)
                elif key == "SD": sendByte(15, value)
                elif key == "AB": sendByte(18, value)
                elif key == "IB": sendByte(9, value)
                elif key == "RS": sendByte(14, value)
                elif key == "IL": sendByte(10, value)
                elif key == "TH": sendByte(16, value)
                elif key == "DB": sendByte(4, value)
                elif key == "AL": sendByte(1, value)
                elif key == "GF": sendByte(40, value)
                elif key == "FP": sendByte(37, value)
                elif key == "ER": sendByte(39, value)
                elif key == "GL": sendByte(43, value)
                elif key == "SL": sendByte(42, value)
                elif key == "DL": sendByte(41, value)
                elif key == "HL": sendByte(5, value)
                elif key == "RL": sendByte(6, value)
                last_sent[key] = value

        

        # print("Checking UDP...")

        try:
            data2, addr2 = udp_read_socket.recvfrom(18889)
            print(data2)
            if len(data2) >= 16:
                locoStatus = format(data2[15], '08b')

                PB, WS, PC, SS, AP, AW, HS, BS = \
                    locoStatus[7], locoStatus[6], locoStatus[5], locoStatus[4], \
                    locoStatus[3], locoStatus[2], locoStatus[1], locoStatus[0]

                msg = f"PB:{PB},WS:{WS},PC:{PC},SS:{SS},AP:{AP},AW:{AW},HS:{HS},BS:{BS}\n"
                print("message: ", msg)
                ser.write(msg.encode('ascii'))

        except BlockingIOError:
            # No UDP packet available right now — totally fine
            pass
        except Exception as e:
            print("UDP error:", e)

except KeyboardInterrupt:
    print("Program manually stopped.")
except Exception as err:
    print("critical error occured.")
    print(err)
finally:
    udp_socket.close()    


