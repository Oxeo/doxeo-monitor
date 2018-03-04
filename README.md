# Web connected thermostat with Raspberry Pi

![Home](/pictures/web_presentation.png)

## Requirements

* Qt 5 (Qtbase, QtSerialPort and Qtscript)
* Mysql

## Installation

* Generate a MakeFile: qmake
* Compile the application: make
* Configure the application: ./doxeo-monitor --configure
* Create a new user: ./doxeo-monitor --user
* Run : ./doxeo-monitor
* Go the web page: http://localhost:8080
* To get somme help : ./doxeo-monitor --help

## Work also with Doxeo-Android

![Home](/pictures/android_presentation2.png)
![Home](/pictures/android_presentation.png)

## Extras

### Install Mysql driver on windows

 1. Download the driver [here](https://dev.mysql.com/downloads/connector/c/).
 2. Copy libmysql.dll into C:\Program Files\Qt\5.5\mingw492_32\bin
 
 ### Install SSL driver on windows

 1. Download Win32 OpenSSL v1.0.2n [here](http://slproweb.com/products/Win32OpenSSL.html).
 2. Install openSLL
 2. Copy libeay32.dll and ssleay32.dll into your executable folder
 
 ### Install SSL driver on raspbian
```
 sudo apt-get install openssl
 ```

### Install Qt 5.5

 1. Download Qt [here](http://download.qt.io/archive/qt/5.5/5.5.1/single/qt-everywhere-opensource-src-5.5.1.tar.xz).
 2. Copy the folder qtbase into your raspberry pi
 4. create a folder qtbase-build next to qtbase folder
 3. go to qtbase-build folder
 3. configure the build
```
../qtbase/configure \
 -v -opengl es2 \
 -device linux-rasp-pi-g++ \
 -device-option CROSS_COMPILE=/usr/bin/ \
 -opensource -confirm-license \
 -optimized-qmake -reduce-exports -force-pkg-config \
 -nomake examples -no-compile-examples \
 -release -qt-pcre -openssl -prefix /opt/Qt5.5
```
 5. do: make
 6. do: sudo make install

You now have Qt installed in /opt/Qt5.5 ready to be used. To configure your Qt project(s) to build with this version run qmake from the installation directory:
```
/opt/Qt5.5/bin/qmake
```
 
### Install QtSerialPort (5.5)

 1. Download Qt [here](http://download.qt.io/archive/qt/5.5/5.5.1/single/qt-everywhere-opensource-src-5.5.1.tar.xz).
 2. Copy the folder qtserialport into your raspberry pi
 3. Create a folder qtserialport-build next to the repository qtserialport
 4. Go to the folder qtserialport-build and do:
```
 qmake ../qtserialport/qtserialport.pro
 make
 sudo make install
```
 
### Install Qtscript

 1. Download Qt [here](http://download.qt.io/archive/qt/5.5/5.5.1/single/qt-everywhere-opensource-src-5.5.1.tar.xz).
 2. Copy the folder qtscript into your raspberry pi
 3. Create a folder qtscript-build next to the repository qtscript
 4. Go to the folder qtscript-build and do:
```
 qmake ../qtscript/qtscript.pro
 make
 sudo make install
```
 
### Allowing your linux userid permission to use your usb device

 1. ls -la /dev/ttyUSB0
 2. sudo usermod -a -G dialout myUserName
 
### Run Doxeo at startup

 1. do: sudo vim /etc/rc.local
 2. add: su pi -c '/home/pi/apps/doxeo-monitor/doxeo-monitor --path /home/pi/apps/doxeo-monitor'
 
### Send mail with raspberry

http://www.pihomeserver.fr/2015/08/13/envoyer-un-email-depuis-votre-raspberry-pi/

### Install ddclient for DynHost OVH

1. sudo apt-get install ddclient
2. sudo vim /etc/ddclient.conf
```
protocol=dyndns2
use=web, web=checkip.dyndns.com
server=www.ovh.com
login=nomdedomaine.net-login
password='mypassword'
dyn.nomdedomaine.net
```
3. sudo service ddclient restart

