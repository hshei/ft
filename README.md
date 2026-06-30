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

**Compress during transfer:**

```bash
./ft -c send bigfile.log          # default compression level
./ft -c9 send bigfile.log         # max compression (slowest)
./ft -c1 send bigfile.log         # fastest compression
```

Compression trades CPU for bandwidth. Useful on slow links; skip it on fast local networks where raw transfer is faster.

**Encrypt during transfer:**

```bash
./ft -e send secret.pdf           # prompts for a password on both ends
./ft -e receive                   # enter the same password
```

Both sides enter a shared password. Files are encrypted with AES-256-CBC before leaving the sender and decrypted on arrival. A wrong password fails the transfer instead of writing garbage.

**Combine compression and encryption:**

```bash
./ft -c9 -e send bigfile.log
```

Data is compressed first, then encrypted.

## How It Works

1. Receiver starts listening for TCP connections and UDP discovery broadcasts
2. Sender broadcasts a UDP discovery packet on the local network — or connects directly if an IP is provided
3. Receiver responds, sender connects via TCP
4. Sender streams file header (filename + size) followed by file data in 32KB chunks
5. Receiver writes to disk with a live progress bar

With `-c`, each chunk is compressed with zlib before sending and decompressed on arrival; compressed chunks are length-prefixed so the receiver knows how many bytes to read. With `-e`, a key is derived from the shared password (PBKDF2), and each chunk is encrypted with AES-256-CBC using a fresh random IV. When both are used, data is compressed then encrypted on the way out, and decrypted then decompressed on the way in.

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

Produces a single binary `ft`. Requires zlib (`-lz`) and OpenSSL (`-lcrypto`), both standard on macOS and Linux. On Linux you may need `libssl-dev`.

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
│   ├── crypto.c       AES-256 encryption, PBKDF2 key derivation
│   └── helper.c       ft_error, format_size
├── lib/
│   └── datastructures/  type-generic data structures library (vector for file lists)
├── Makefile
└── .gitignore
```

## TODO

- Transfer resume on interrupted connections
