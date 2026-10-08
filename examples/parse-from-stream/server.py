#!/usr/bin/env python3
import socket
import threading
import random
import time
import struct
from alive_progress import alive_bar

# ------------------------------------------------------------------
# Configuration
# ------------------------------------------------------------------
NUM_OBJECTS   = 300
NUM_EVENTS    = 10_0000
INFINITE      = True

# How long to sleep between blocks (seconds).
# 0.0 = send as fast as possible
SEND_DELAY    = 0.0

BIND_IP       = "0.0.0.0"
BIND_PORT     = 5555
BASE_ADDR     = 0x20000000

# Timestamp behaviour (like a real TraceX timer)
TS_START      = 1000          # initial value
TS_STEP_MIN   = 1             # minimum ticks between events
TS_STEP_MAX   = 5             # maximum ticks between events (small jitter)
# ------------------------------------------------------------------
# Structure sizes
# ------------------------------------------------------------------
HEADER_SIZE = 48
OBJECT_SIZE = 48
EVENT_SIZE  = 32
NAME_SIZE   = 32

def pack_u32(v): return struct.pack("<I", v & 0xFFFFFFFF)
def pack_u16(v): return struct.pack("<H", v & 0xFFFF)
def pack_u8(v):  return struct.pack("<B", v & 0xFF)

def make_name(s: str) -> bytes:
    b = s.encode("ascii", errors="replace")[:NAME_SIZE-1]
    return b + b"\x00" * (NAME_SIZE - len(b))

def build_header(num_objects: int, num_events: int) -> bytes:
    registry_start = BASE_ADDR + HEADER_SIZE
    registry_end   = registry_start + num_objects * OBJECT_SIZE
    buffer_start   = registry_end
    buffer_end     = buffer_start + num_events * EVENT_SIZE
    current_ptr    = buffer_start

    buf = bytearray()
    buf += pack_u32(0x54585442)          # TXTB
    buf += pack_u32(0xFFFFFFFF)          # timer mask (32-bit)
    buf += pack_u32(BASE_ADDR)
    buf += pack_u32(registry_start)
    buf += pack_u16(0)
    buf += pack_u16(NAME_SIZE)
    buf += pack_u32(registry_end)
    buf += pack_u32(buffer_start)
    buf += pack_u32(buffer_end)
    buf += pack_u32(current_ptr)
    buf += pack_u32(0xAAAAAAAA)
    buf += pack_u32(0xBBBBBBBB)
    buf += pack_u32(0xCCCCCCCC)
    assert len(buf) == HEADER_SIZE
    return bytes(buf)

def generate_objects(num_objects: int):
    object_ptrs = []
    object_types = [
        (1, "Thread"), (2, "Timer"), (3, "Queue"), (4, "Semaphore"),
        (5, "Mutex"), (6, "EventFlags"), (7, "BlockPool"), (8, "BytePool"),
    ]
    buf = bytearray()

    for i in range(num_objects):
        if i > 5 and random.random() < 0.15 and object_ptrs:
            ptr = random.choice(object_ptrs)
            available = 0
        else:
            ptr = 0x1000 + i * 0x40 + random.randint(0, 0x20)
            object_ptrs.append(ptr)
            available = 0

        typ, basename = random.choice(object_types)
        name = f"{basename}_{i}"

        if typ == 1:
            prio = random.randint(0, 31)
            reserved1 = prio & 0xFF
            reserved2 = 0
            param1 = 0x2000 + i * 0x400
            param2 = random.choice([512, 1024, 2048, 4096])
        else:
            reserved1 = reserved2 = 0
            param1 = random.randint(1, 100)
            param2 = random.randint(0, 64)

        buf += pack_u8(available)
        buf += pack_u8(typ)
        buf += pack_u8(reserved1)
        buf += pack_u8(reserved2)
        buf += pack_u32(ptr)
        buf += pack_u32(param1)
        buf += pack_u32(param2)
        buf += make_name(name)

    assert len(buf) == num_objects * OBJECT_SIZE
    return bytes(buf), object_ptrs

def generate_events(num_events: int, object_ptrs: list, start_ts: int):
    """
    Generate events with a strictly increasing timestamp.
    Returns (event_bytes, next_ts) so the caller can continue
    the timeline in the next block.
    """
    event_ids = [1, 2, 3, 4, 5, 10, 11, 12, 13, 14, 20, 21,
                 30, 31, 40, 41, 50, 51, 100, 101, 4096]
    buf = bytearray()
    ts = start_ts

    for _ in range(num_events):
        r = random.random()
        if r < 0.05:
            thread_ptr = 0xFFFFFFFF
            priority   = random.randint(0, 0xFFFFFFFF)
        elif r < 0.07:
            thread_ptr = 0xF0F0F0F0
            priority   = 0
        else:
            thread_ptr = random.choice(object_ptrs) if object_ptrs else 0x1000
            priority   = (random.randint(0, 31) << 16) | random.randint(0, 31)

        event_id = random.choice(event_ids)

        # Steady incremental timestamp (small jitter only)
        ts += random.randint(TS_STEP_MIN, TS_STEP_MAX)

        info1 = random.choice(object_ptrs) if object_ptrs else random.randint(0, 0xFFFF)
        info2 = random.randint(0, 0xFFFFFFFF)
        info3 = random.randint(0, 0xFFFFFFFF)
        info4 = random.randint(0, 0xFFFFFFFF)

        buf += pack_u32(thread_ptr)
        buf += pack_u32(priority)
        buf += pack_u32(event_id)
        buf += pack_u32(ts)          # increasing timestamp
        buf += pack_u32(info1)
        buf += pack_u32(info2)
        buf += pack_u32(info3)
        buf += pack_u32(info4)

    assert len(buf) == num_events * EVENT_SIZE
    return bytes(buf), ts          # return the last used timestamp

def handle_client(client_socket):
    try:
        print("[+] Building header ...")
        header = build_header(NUM_OBJECTS, NUM_EVENTS)
        block_size = (NUM_OBJECTS * OBJECT_SIZE) + (NUM_EVENTS * EVENT_SIZE)
        print(f"[+] Header ready – {NUM_OBJECTS} objects, {NUM_EVENTS} events "
              f"(block size = {block_size} bytes)")

        # Send header once
        client_socket.sendall(header)
        print("[+] Header sent")

        total_bytes = 0
        blocks = 0
        current_ts = TS_START          # <-- persistent timestamp across blocks

        bar_total = block_size if not INFINITE else None

        with alive_bar(bar_total, title="Sending TraceX data", unit="B") as bar:
            while True:
                # Generate one full block (objects + events)
                objects, object_ptrs = generate_objects(NUM_OBJECTS)
                events, current_ts = generate_events(NUM_EVENTS, object_ptrs, current_ts)

                # Send the whole block
                client_socket.sendall(objects)
                client_socket.sendall(events)

                sent = len(objects) + len(events)
                total_bytes += sent
                blocks += 1

                bar(sent)
                bar.text(f"blocks={blocks}  total={total_bytes} B  ts={current_ts}")

                if not INFINITE:
                    break

                if SEND_DELAY > 0:
                    time.sleep(SEND_DELAY)

        print(f"[+] Finished – sent {blocks} block(s), {total_bytes} bytes")

    except (BrokenPipeError, ConnectionResetError, ConnectionAbortedError):
        print("[+] Client disconnected")
    except Exception as e:
        print(f"[!] Error: {e}")
    finally:
        try:
            client_socket.close()
        except Exception:
            pass

def main():
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind((BIND_IP, BIND_PORT))
    server.listen(1)
    print(f"[+] Listening on {BIND_IP}:{BIND_PORT}")
    print(f"[+] SEND_DELAY = {SEND_DELAY} s   INFINITE = {INFINITE}")

    try:
        while True:
            client, addr = server.accept()
            print(f"[+] Accepted connection from {addr[0]}:{addr[1]}")
            t = threading.Thread(target=handle_client, args=(client,), daemon=True)
            t.start()
    except KeyboardInterrupt:
        print("\n[+] Server stopped")
    finally:
        server.close()

if __name__ == "__main__":
    main()