# sfs
**Simple File Sharing (SFS)** is a tool to share files/folders over the wire. It's a quick-and-dirty way to share files, compared to more sophisticated way in the likes of SFTP or NFS. It leverages [gRPC](https://grpc.io/) for communication between server and client.

[![CMake on a single platform](https://github.com/cynthia2006/sfs/actions/workflows/cmake-single-platform.yml/badge.svg)](https://github.com/cynthia2006/sfs/actions/workflows/cmake-single-platform.yml)

## Goals
- ncurses based GUI for client.
- Support TLS for encrypted file transfers.
- Sandboxing in the server.
- Parallel file transfers.
