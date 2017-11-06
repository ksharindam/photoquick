# QmageView
A simple image viewer with some useful features (written in qt4).

### Description
This program is aimed at ease of use, quick opening, and doing most necessary features.  
This can ..  
 * Resize
 * Crop in particular ratio
 * Rotate
 * Create photo grid for printing
 * Add Border

This image viewer is tested on Raspberry Pi (Raspbian).  
N.B - You can use the python version of this program [pypicview](https://github.com/ksharindam/pypicview)

### Build
To build this program, extract the source code zip.  
Open terminal and change directory to qmageview/QmageView.  
**Install dependencies ...**  
 * libqt4-dev  

Then run these commands to compile...  
```
qmake  
make -j4  
```

**Runtime Dependencies**
* libqtcore4
* libqtgui4
* libqt4-svg
### Install
To install just copy the binary to /usr/local/bin folder (or run this command).  
`sudo install qmageview /usr/local/bin`  

To uninstall run...  
`sudo rm /usr/local/bin/qmageview`  

### Usage
To run this program...
`qmageview`

To open image.jpg with it...  
`qmageview image.jpg`  
