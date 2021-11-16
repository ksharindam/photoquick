![License](https://img.shields.io/github/license/ksharindam/photoquick)
![Release](https://img.shields.io/github/v/release/ksharindam/photoquick)
![Release Date](https://img.shields.io/github/release-date/ksharindam/photoquick)
![Downloads Total](https://img.shields.io/github/downloads/ksharindam/photoquick/total)

# PhotoQuick (for Linux and Windows)
A simple handy image viewer and editor with some useful features (written in qt4).

### Description
This program is aimed at ease of use, quick opening, and doing most necessary features.  

 * Export to PDF  
 * Auto Resize to file size  
 * Crop in particular ratio  
 * Rotate, mirror, perspective transform  
 * Add Border  
 * Create photo grid for printing  
 * Magic Eraser (inpainting)  
 * Intelligent Scissor  
 * Filters  (Scan Page, Reduce Noise, Auto Contrast, White Balance )  
 * Photo Optimizer & Batch Resize  


### Download
Download the precompiled packages from [releases page](https://github.com/ksharindam/photoquick/releases).  
For Windows download .exe package and install it.  
For Linux download .AppImage package, mark it executable, and double click to run.  

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
Add Qt/4.8.7/bin directory and mingw32/bin directory in PATH environment variable.  
In src directory open Command Line.  
Run command...  
`qmake`  
`make -j4`  

### Plugins
The plugins/ directory contains only sample plugins.  
**Build (Linux and Windows) :**  
Open terminal or command line in project root directory.  
Then run these commands to compile...  
```
cd plugins  
qmake  
make -j4  
```  
**Install (Linux) :**  
`sudo make install`  

Get more plugins from https://github.com/ImageProcessing-ElectronicPublications/photoquick-plugins  
Also you can create your own plugins and use with it.  

### Usage
To run this program...  
`photoquick`  

To open image.jpg with it...  
`photoquick image.jpg`  

### Keyboard Shortcuts
Reload Image : R  
Delete Image : Delete  

### Supported Image Formats
All formats supported by Qt are supported in this program.  
**Read :** JPG, PNG, GIF, SVG, TIFF, ICO, BMP, XPM, XBM, PPM, PBM, PGM  
**Write :** JPG, PNG, TIFF, ICO, BMP, XPM, XBM, PPM  

JPEG2000 and WebP formats are supported via image format plugins.  
You can get these here...  
https://github.com/ksharindam/qt4-imageformat-plugins  

### Screenshots

Main Window  
![Main Window](data/screenshots/Screenshot1.jpg)  

Photo-Grid  
![Photo Grid](data/screenshots/Screenshot2.jpg)  

Scissor Tool  
![Scissor Tool](data/screenshots/Screenshot3.jpg)  

