# Project Title

hanmatek user interface.

## Getting Started

These instructions will get you  running hanmatek program in order to command the supply power.

### Prerequisites

The things you need before installing the software.

* glib-2.0
```
sudo apt install libglib2.0-dev
```
* readline
```
sudo apt install libreadline6-dev
```

### Installation

By running make all, the application will be compiled and user access to the serial port is attributed.

```
$ sudo make all
```

## Usage

The application is based on the Main Event Loop of the glib library, so we have an iteractive menu.

```
$ sudo ./hanmatek
```
A list of the supported command is accessible with the help command 
```
> help
help                             Show this help
Exit                             Exit interactive mode
Power-Switch          <0/1>      Switch on/off
Set-Voltage         <voltage>    Set Voltage to a defined value in Volts
Protect-Voltage     <voltage>    Protect Voltage from being changed
```

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

