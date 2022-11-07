# seq192

> :warning: Build shared branch  
> `make -f Makefile.shared`  
> `make -f Makefile.shared standalone`  
> `cd src && ./seq192`  
>   
> There's a nasty GTK segfault but doesn't matter much though...  


MIDI sequencer based on seq24 with less features and more swag.

**Less features**
- No song editor
- No keyboard controls
- No midi controls
- Linux only

**More swag**
- Interface rewritten with GTK3
- OSC controls
- almost 192 patterns per set

![Seq192 main window](https://user-images.githubusercontent.com/5261671/133999743-445a4285-a1b6-400a-a6cd-9bce6714f8a2.png)

![Seq192 edit window](https://user-images.githubusercontent.com/5261671/133999740-bc7f57e5-4c3d-4496-98cf-3df88d8b7a48.png)



## Build

**Dependencies** (as debian packages)
```
libjack-jackd2-dev liblo-dev libgtkmm-3.0-dev libasound2-dev nlohmann-json3-dev
```

**Build**
```
make clean && make -j8
```



**Run**

```
usage: ./src/seq192 [options]

options:
  -h, --help              show available options
  -f, --file <filename>   load midi file on startup
  -c, --config <filename> load config file on startup
  -p, --osc-port <port>   osc input port (udp port number or unix socket path)
  -j, --jack-transport    sync to jack transport
  -n, --no-gui            enable headless mode
  -v, --version           show version and exit
```

**Install**

```bash
sudo make install
```

Append `PREFIX=/usr` to override the default installation path (`/usr/local`)

**Uninstall**

```bash
sudo make uninstall
```

Append `PREFIX=/usr` to override the default uninstallation path (`/usr/local`)

## Documentation

See [seq192.ammd.net](https://seq192.ammd.net/) or run `man seq192` after installing.

## Web UI

A web UI built with [Open Stage Control](https://openstagecontrol.ammd.net/) is available at https://github.com/jean-emmanuel/seq192-control
