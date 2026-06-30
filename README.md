# ft

A local network file transfer tool written in C. Send files between machines with one command — no setup, no accounts, no cloud.

## Usage

**Receive files:**

```bash
./ft receive
```

**Send files (auto-discover receiver on the network):**

```bash
./ft send photo.jpg
./ft send file1.txt file2.pdf file3.jpg
```

**Send files (manual IP):**

```bash
./ft send 192.168.1.5 photo.jpg
./ft send 192.168.1.5 file1.txt file2.pdf
```

**Send a directory (recursive):**

```bash
./ft send myfolder
```

The folder structure is preserved on the receiver side.

## How It Works

1. Receiver starts listening for TCP connections and UDP discovery broadcasts
2. Sender broadcasts a UDP discovery packet on the local network — or connects directly if an IP is provided
3. Receiver responds, sender connects via TCP
4. Sender streams file header (filename + size) followed by file data in 4KB chunks
5. Receiver writes to disk with a live progress bar

```
sending 2 file(s)

sent notes.txt (1.2 KB)
sent backup.tar (45.3 MB)

done — 2 file(s) sent
```

```
waiting for connection...
discovered by 192.168.1.10
connected from 192.168.1.10
receiving 2 file(s)

receiving notes.txt (1.2 KB)
[##############################] 100%  1.2 KB  1.2 KB/s  ETA 0s
saved notes.txt (1.2 KB)

receiving backup.tar (45.3 MB)
[######################        ] 73%  33.1 MB  16.5 MB/s  ETA 1s
```

## Build

```bash
make
```

Produces a single binary `ft`. No external dependencies — just POSIX sockets and standard C.

## Protocol

Binary header per file, followed by raw file data:

```
┌────────────────┬──────────────────┬───────────────┬──────────────┐
│ name_len (2)   │ filename (n)     │ filesize (8)  │ file data... │
└────────────────┴──────────────────┴───────────────┴──────────────┘
```

Auto-discovery uses UDP broadcast on the local network. The receiver listens on both a TCP port (for file transfer) and a UDP port (for discovery) simultaneously using `poll`.

## Project Structure

```
ft/
├── include/
│   ├── sender.h       sender interface
│   ├── receiver.h     receiver interface
│   ├── discovery.h    UDP auto-discovery
│   └── helper.h       error handling, formatting
├── src/
│   ├── main.c         argument parsing, directory walking, mode dispatch
│   ├── sender.c       connect, stream files
│   ├── receiver.c     listen, receive files, recreate directories, progress bar
│   ├── discovery.c    UDP broadcast/listen with poll
│   └── helper.c       ft_error, format_size
├── lib/
│   └── datastructures/  type-generic data structures library (vector for file lists)
├── Makefile
└── .gitignore
```

## TODO

- Transfer resume on interrupted connections
- Encryption (TLS)
- Compression
