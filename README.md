# Cairo System Dynamics

(c) 2017-2018 Nuno Ferreira

License: GNU General Public License version 3 or later

## About

This is a general purpose simulator written in GTK3+.

It can simulate gases, biological systems, newtonian gravity... 

## Building

### Install dependencies:

sudo apt-get install build-essential libcairo2-dev libgtk-3-dev

### Compile & run:

```gcc Cairo_System_Dynamics_0.15.c -lm -o Cairo_System_Dynamics `pkg-config --cflags --libs gtk+-3.0` && ./Cairo_System_Dynamics```

