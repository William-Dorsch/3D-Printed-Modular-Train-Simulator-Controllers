import serial
import keyboard
import time

# com configuration

com_port = input("enter COM port: ")
# com_port = "COM3" # changes based on a number of factors, check with arduino IDE
baud_rate = 9600

# initialize 
current_throttle = 0
current_brake = 0
current_reverser = 0
current_bell = 0
current_horn = 0
current_sand = 0
current_alerter = 0
current_HL = 3
current_RL = 3
current_EM = 0

# open serial port
ser = serial.Serial(com_port, baud_rate)

# function for press and release of a key (for the throttle and brakes)
def pressKey(key):
    keyboard.press(key)
    time.sleep(0.01)
    keyboard.release(key)
    time.sleep(0.01)

# try loop for error handling without program crashes
try:
    # main logic loop
    while True:
        # read line from serial
        serial_data = ser.readline().decode(errors='ignore').strip()
        
        # ignores empty lines
        if not serial_data:  
            continue  
        
        # remnoves leading comma, but still handles data if there is no leading comma
        if serial_data[0] == ',':
            serial_data = serial_data[1:]

        # split data into key, value pairs with the two letter identifier as the key and the integer value as the value
        parsed_data = {}
        try:
            sections = serial_data.split(',')
            for section in sections:
                key, value = section.split(':')
                parsed_data[key] = int(value)  # Convert value to integer
        except ValueError:
            print("bad line, data skipped:", serial_data)
            continue

        # setting individual variables equal to the parsed data
        target_horn = parsed_data.get("HR", 0)
        target_bell = parsed_data.get("BL", 0)
        target_sand = parsed_data.get("SD", 0)
        if parsed_data.get("AB", 0) <= 252:
            target_brake = round(parsed_data.get("AB", 0) / 12)
        else:
            target_brake = 21
        if parsed_data.get("AB", 0) == 255:
            target_EM = 1
        else:
            target_EM = 0
        target_reverser = parsed_data.get("RS", 0)
        target_throttle = parsed_data.get("TH", 0)
        target_alerter = parsed_data.get("AL", 0)
        target_HL = parsed_data.get("HL", 0)
        target_RL = parsed_data.get("RL", 0)

        # throttle
        if current_throttle != target_throttle:
            if target_throttle > current_throttle:
                pressKey('w')
                current_throttle += 1
            if target_throttle < current_throttle:
                pressKey('s')
                current_throttle -= 1
        
        # brakes
        if current_brake != target_brake:
            if target_brake > current_brake:
                pressKey('e')
                current_brake += 1
            if target_brake < current_brake:
                pressKey('q')
                current_brake -= 1

        # horn
        # can this be made into a function?  I guess we'll never know
        if target_horn == 1 and current_horn == 1:
            keyboard.press('n')
        elif target_horn == 1 and current_horn == 0:
            keyboard.press('n')
            current_horn += 1
        elif target_horn == 0 and current_horn == 1:
            keyboard.release('n')
            current_horn -= 1

        # bell
        if target_bell == 1 and current_bell == 1:
            keyboard.press('b')
        elif target_bell == 1 and current_bell == 0:
            keyboard.press('b')
            current_bell += 1
        elif target_bell == 0 and current_bell == 1:
            keyboard.release('b')
            current_bell -= 1

        # sand
        if target_sand == 1 and current_sand == 1:
            keyboard.press('shift')
        elif target_sand == 1 and current_sand == 0:
            keyboard.press('shift')
            current_sand += 1
        elif target_sand == 0 and current_sand == 1:
            keyboard.release('shift')
            current_sand -= 1

        # doors
        if target_alerter == 1 and current_alerter == 1:
            keyboard.press('t')
        elif target_alerter == 1 and current_alerter == 0:
            keyboard.press('t')
            current_alerter += 1
        elif target_alerter == 0 and current_alerter == 1:
            keyboard.release('t')
            current_alerter -= 1

        # emergency brake
        if target_EM == 1 and current_EM == 0:
            keyboard.press('backspace')
            current_EM += 1
        elif target_EM == 0 and current_EM == 1:
            keyboard.release('backspace')
            current_EM -= 1

        # headlight handling
        # note for future me: whatever the headlight dials are set to at the end of one run, they will initialize as the next run
        # so keep the dials the same between runs please
        # thanks

        if target_HL == 2:
            target_HL = 1
        if target_HL == 3:
            target_HL = 2

        # reminder: 0 = off, 1 = dim, 2 = on
        # the order being off -> on -> dim
        # making the numbers: 0 -> 2 -> 1
        if target_HL == 1 and current_HL == 0:
            pressKey('l')
            pressKey('l')
            current_HL += 1
        elif target_HL == 0 and current_HL == 1:
            pressKey('l')
            current_HL -= 1
        if target_HL == 2 and current_HL == 1:
            pressKey('l')
            pressKey('l')
            current_HL += 1
        elif target_HL == 1 and current_HL == 2:
            pressKey('l')
            current_HL -= 1
        if target_HL == 3 and current_HL == 2:
            pressKey('l')
            pressKey('l')
            current_HL += 1
        elif target_HL == 2 and current_HL == 3:
            pressKey('l')
            current_HL -= 1

        if target_RL == 2:
            target_RL = 1
        if target_RL == 3:
            target_RL = 2

        if target_RL == 1 and current_RL == 0:
            pressKey('k')
            current_RL += 1
        elif target_RL == 0 and current_RL == 1:
            pressKey('k')
            pressKey('k')
            current_RL -= 1
        if target_RL == 2 and current_RL == 1:
            pressKey('k')
            current_RL += 1
        elif target_RL == 1 and current_RL == 2:
            pressKey('k')
            pressKey('k')
            current_RL -= 1
        if target_RL == 3 and current_RL == 2:
            pressKey('k')
            current_RL += 1
        elif target_RL == 2 and current_RL == 3:
            pressKey('k')
            pressKey('k')
            current_RL -= 1
except:
    print("an error has occured.  what error?  no clue. did you hit ctrl c in the terminal by chance?")

