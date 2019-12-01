# QmageView (for Linux and Windows)
A simple image viewer with some useful features (written in qt4).

### Description
This program is aimed at ease of use, quick opening, and doing most necessary features.  

 * Resize  
 * Crop in particular ratio  
 * Rotate  
 * Create photo grid for printing  
 * Add Border  
 * Filters  
  * Grayscale  
  * Scan Page  
  * Blur  
  * Sharpen  
  * Auto Contrast  
  * White Balance
  * Despeckle  
  * Reduce Noise  

This image viewer is tested on Raspberry Pi (Raspbian).  

### Build (Linux)
Install dependencies...  
**Build dependencies ...**  
 * libqt4-dev  

To build this program, extract the source code zip.  
Open terminal and change directory to src/  
Then run these commands to compile...  
```
qmake  
make -j4  
```

To install run ...  
`sudo make install`  

To uninstall, run ...  
`sudo make uninstall`  

**Runtime Dependencies**  
* libqtcore4  
* libqtgui4  
* libqt4-svg  (optional for svg support)  
* libgomp1

### Build (Windows)
Download Qt 4.8.7 and minGW32  
add Qt/4.8.7/bin directory and mingw32/bin directory in PATH environment variable.  
In qmageview/src directory open Command Line.  
Run command...  
`qmake`  
`make -j4`  

You can download the precompiled windows exe package in the [release page](https://github.com/ksharindam/qmageview/releases).  

### Usage
To run this program...  
`qmageview`  

To open image.jpg with it...  
`qmageview image.jpg`  
