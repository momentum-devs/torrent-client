# Torrent Client

This is a simple BitTorrent client written in C++ and Boost Asio.

Quick start
==========================
If you want to try this BitTorrent client yourself, follow the steps below:

- Clone this repository, and make sure that cmake (> 3.23) is installed.

```console
$ git clone https://github.com/momentum-devs/torrent-client.git
$ cd torrent-client && mkdir build && cd build
$ cmake .. && make -j
```

- Initialize git submodules:

```console
$ git submodule update --init --recursive
$ cd torrent-client && mkdir build && cd build
$ cmake .. && make -j
```

- Build source code:

```console
$ cd torrent-client && mkdir build && cd build
$ cmake .. && make -j
```

- To download the debian iso as an example, enter the following command:

```console
$ ./torrent-client --torrent_file ../torrents/debian.torrent --destination_directory ../torrents
```

- All files in the `torrents` directory can be used for testing.

Commandline Options
==========================
The program supports the following commandline options:

| Options | Alternative             | Description                                               | Default              |
|---------|-------------------------|-----------------------------------------------------------|----------------------|
| --t     | --torrent-file          | Path to the torrent file                                  | REQUIRED             |
| --d     | --destination_directory | The output directory to which the file will be downloaded | REQUIRED             |

Supported Features
==========================
The current implementation of this BitTorrent client only supports the following features:

- Downloading single-file torrents in a multithreaded manner.
- Downloading multi-file torrents in a multithreaded manner.
- Resuming a download.
- Connecting to as many peers as possible.
