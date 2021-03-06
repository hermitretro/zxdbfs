
# Virtual Filesystem for ZXDB

ZXDBFS provides a standard-looking filesystem on UNIX hosts that presents
data from ZXDB (the online database for Sinclair ZX machines). ZXDBFS
enables any UNIX-based application to interface with ZXDB (i.e., download
games) over the network without any specific application code. The filesystem
is read-only.

The implementation uses [FUSE](https://github.com/libfuse/libfuse),
[ZXDB REST API](https://api.zxinfo.dk/v3/),
[libcurl](https://curl.se/libcurl/) and
[json-c](https://github.com/json-c/json-c).

# Current Status

This is a very experimental repository. Currently working:

* Directory listing with attributes
* File read (cp, cat, etc....)
* Magic Search functionality
* Magic Status functionality
* Caching

# Building and running

ZXDBFS has some system requirements.

```
% cd $HOME
% sudo apt-get install -y g++ git cmake libcurl4-openssl-dev
% sudo apt-get install -y googletest
% git clone https://github.com/hermitretro/zxdbfs.git
```

This project has a dependency on `libfuse3`. You should install that first
via whichever package manager (or source build) you prefer. If you need to
build from source, follow the instructions with `libfuse3`.

```
% sudo apt-get install pkg-config meson
```

* For Ubuntu 18.04, you will need to build from [source](https://github.com/libfuse/libfuse)
* For Ubuntu 20.04, you can install via `sudo apt-get install -y libfuse3-dev`
* For other Linuxes/BSD, you will likely need to build from source
* macOS and Windows are not supported

This project builds using cmake:

```
% cd $HOME/zxdbfs
% mkdir build
% cd build
% cmake ..
% make
```

## libfuse3 filesystem

That should result in an executable `zxdbfsd` in the `build` directory.

To run it:

```
% zxdbfsd mountpoint
```

where `mountpoint` should be an existing directory. By default, `zxdbfsd`
will background. You can unmount the filesystem with:

```
% fusermount3 -u mountpoint
```

You can run `zxdbfsd` in the foreground via:

```
% zxdbfsd -d mountpoint
```

which is also handy for debugging and killing directly with `Ctrl-C`.

The directory you mount onto will not be destroyed, but it will be unavailable
whilst `zxdbfsd` runs. It will become available once `zxdbfsd` is
unmounted or exits.

# Using the filesystem

## Throttling (Not yet implemented)

The filesystem will throttle automatically to avoid heavy load onto ZXDB.
For example, we recommend NOT using commands such as `find`
or `tree`. These will execute deep-trawls on the filesystem and make
a large number of requests to ZXDB.

## UNIX Commands

Standard UNIX commands will interact with the filesystem and present
data as expected. Be careful with deep-trawl commands such as `tree`
and `find`.

## Structure

The filesystem is organised with three top-level directories.

### /by-letter

The `by-letter` root directory, when listed, will present a set of
sub-directories of single letters and numbers. Traversing to a subdirectory
will list programs starting with the subdirectory name.

The directory names are presented as `filename_ZXDBID`. Each directory
corresponds to a program entry in ZXDB.

### /search

The `/search` root directory is "magic" and allows you to query ZXDB via a
file-like interface.

For example, to search for "Gargoyle Games" you should just append the search
term to the path, e.g., `/search/Gargoyle\ Games`. This will then execute
the search and return the games as directories under `/search/Gargoyle\ Games`.

To see which searches have already occurred, simply list the directories in
`/search`.

### /status

The `/status` root directory provides three files that can be used to
interrogate the status of the WiFi subsystem and ZXDBFS.

`/status/json` is a magic file containing the WiFi configuration from 
`wpa_cli status` plus process information on whether `ntpd`, 
`spi-fat-fuse` and `zxdbfsd` are running in JSON format. This mode also
performs a call to ZXDB to retrieve the version number.

`/status/text` is a magic file containing the WiFi configuration from
`wpa_cli status` plus process information on whether `ntpd`, 
`spi-fat-fuse` and `zxdbfsd` are running in a plain text format. This mode
also performs a call to ZXDB to retrieve the version number.

`/status/summary` will return a file containing either "0" or "1". "0"
will be returned if `ntpd`, `zxdbfsd` and `spi-fat-fuse` are running
plus the `wpa_state` from `wpa_cli status` is `COMPLETED`. If any of
these are not true, "1" will be returned.

### Hidden utility directories

There are some hidden utility directories that will never appear in the
filesystem listings.

#### /cache/fscache

Display the contents of the filesystem cache

#### /cache/fscache/flush

Wipe the filesystem cache

#### /cache/urlcache/flush

Flush the URL cache

# Licensing

* [json-c LICENSE](https://github.com/json-c/json-c/blob/master/COPYING)
* Everything else within this package is covered by the
  [GNU General Public License V3](LICENSE.txt).
