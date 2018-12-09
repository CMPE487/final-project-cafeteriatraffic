import sys
import time

macs_24 = {}
macs_28 = {}
macs_36 = {}

f = open("maclist.txt", encoding="utf8")
for line in f.read().split('\n'):
  if line == '': continue
  if line[0] == '#': continue
  mac = line.split(' ')[0]
  if len(mac) == 8: # mac24
    num = (int(mac[0:2], 16) << 16) | (int(mac[3:5], 16) << 8) | (int(mac[6:8], 16) << 0)
    macs_24[num] = " ".join(line.split(" ")[1:-1])
  else:
    after = int(mac.split('/')[1])
    mac = mac.split('/')[0]
    if after == 28: # mac28
      num = (int(mac[0:2], 16) << 24) | (int(mac[3:5], 16) << 16) | (int(mac[6:8], 16) << 8) | (int(mac[9:11], 16) << 0)
      macs_28[num] = " ".join(line.split(" ")[1:-1])
    elif after == 36: # mac36
      num = (int(mac[0:2], 16) << 32) | (int(mac[3:5], 16) << 24) | (int(mac[6:8], 16) << 16) | (int(mac[9:11], 16) << 8) | (int(mac[12:14], 16) << 0)
      macs_36[num] = " ".join(line.split(" ")[1:-1])

def get_manufacturer(mac):
  mac_num_24 = (int(mac[0:2], 16) << 16) | (int(mac[3:5], 16) << 8) | (int(mac[6:8], 16) << 0)
  mac_num_28 = (int(mac[0:2], 16) << 24) | (int(mac[3:5], 16) << 16) | (int(mac[6:8], 16) << 8) | (int(mac[9:11], 16) & 0xf0)
  mac_num_36 = (int(mac[0:2], 16) << 32) | (int(mac[3:5], 16) << 24) | (int(mac[6:8], 16) << 16) | (int(mac[9:11], 16) << 8) | (int(mac[12:14], 16) & 0xf0)
  if mac_num_24 in macs_24:
    return macs_24[mac_num_24]
  if mac_num_28 in macs_28:
    return macs_28[mac_num_28]
  if mac_num_36 in macs_36:
    return macs_36[mac_num_36]
  return ""


timestamps = {}
t_diffs = {}
counts = {}

import serial
s = serial.Serial('COM12', 115200, timeout=1)
while True:
  line = s.readline()
  if line == b'': continue
  try:
    print(str(line, 'ASCII').strip())
  except: continue
  if b'Source MAC: ' in line:
    mac = line.split(b'Source MAC: ')[1].strip().split(b' ')[0].strip()
    if len(mac) != 17: continue
    if mac not in timestamps:
      timestamps[mac] = []
      counts[mac] = 0
      t_diffs[mac] = []
    else:
      if time.time() - timestamps[mac][-1] < 5.0: continue ## TOO SOON
      t_diffs[mac].append(time.time() - timestamps[mac][-1])
    timestamps[mac].append(time.time())
    counts[mac] += 1
    x = sys.stdout.write("mac: " + str(mac, 'ASCII') + " ")
    try:
      man = get_manufacturer(mac)
      if (man != ''): x = sys.stdout.write(man + " ")
      if ((int(mac[0:2], 16) | 0x02) & 0xfe) == int(mac[0:2], 16):
        x = sys.stdout.write("Locally Administered! ")
      x = sys.stdout.write("\n")
    except:
      x = sys.stdout.write("exc\n")

sorted_counts = sorted(counts.items(), key=lambda kv: kv[1])
# manufacturer_counts = [(get_manufacturer(x[0]), x[1]) for x in sorted_counts]

for i in range(1, 11):
  p = sorted_counts[-i]
  print("###################################")
  print("Mac: " + str(p[0], 'ASCII'))
  print("Manufacturer: " + get_manufacturer(p[0]))
  if ((int(mac[0:2], 16) | 0x02) & 0xfe) == int(mac[0:2], 16):
    print("Locally Administered!")
  x = sys.stdout.write("t_diffs: ")
  print(t_diffs[p[0]])