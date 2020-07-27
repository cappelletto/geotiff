/**
 * @file geotiff_test.cpp
 * @author Jose Cappelletto (cappelletto@gmail.com)
 * @brief Geotiff testing sandbox
 * @version 0.1
 * @date 2020-07-04
 * 
 * @copyright Copyright (c) 2020 Licensed under GNU GPLv3
 *
 */

// THIS IS WORK IN PROGRESS
// The goal is to have this library as a submodule capable of providing an easy interface between GeoTIFF files and 
// OpenCV via GDAL. Native OpenCV GDAL driver does not retrieve the TIFF metadata.
// Raster layers are the priority, but eventual vector layer support is planned.
// Please visit projects site: https://github.com/cappelletto/geotiff
// Geotiff object based on Gerasimos' blog here: https://gerasimosmichalitsianos.wordpress.com/2018/11/30/431/

#include <gdal_priv.h>
#include <cpl_conv.h> // for CPLMalloc()
// GDAL specific libraries

///Basic C and C++ libraries
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <cmath>
#include <stdexcept>
#include <vector>

#include "geotiff.hpp"

using namespace std;

const std::string green("\033[1;32m");
const std::string yellow("\033[1;33m");
const std::string cyan("\033[1;36m");
const std::string red("\033[1;31m");
const std::string reset("\033[0m");

int main()
{
/*	PARSER section */
    std::string descriptionString = \
    "geotiff_test - Simple example to test Geotiff class usage \
    Compatible interface with geoTIFF depth/height datasets via GDAL + OpenCV";

    cout << descriptionString << endl;

    cout << cyan << "geotiff_test" << reset << endl; // CREATE OUTPUT TEMPLATE STRING
    cout << "\tGit commit:\t" << yellow << GIT_COMMIT << reset << endl;
    // cout << "\tBuilt:\t" << __DATE__ << " - " << __TIME__ << endl;   // TODO: solve, make is complaining about this

	std::string inputFileName = "input.tif";
    // create the container and the open input file
    Geotiff geoContainer (inputFileName.c_str());
    if (!geoContainer.isValid()){ // check if nothing wrong happened with the constructor
        cout << red << "Error opening Geotiff file: " << reset << inputFileName << endl;
        return -1;
    }

    //**************************************
    // Get/print summary information of the TIFF 
    GDALDataset *poDataset;
    poDataset = geoContainer.GetDataset(); //pull the pointer to the main GDAL dataset structure
    geoContainer.ShowInformation(); // show detailed info if asked for

	return 0;

}


/* gdalinfo test.tiff output:

Driver: GTiff/GeoTIFF
Files: test.tif
Size is 2195, 1853
Coordinate System is:
GEOGCS["WGS 84",
    DATUM["WGS_1984",
        SPHEROID["WGS 84",6378137,298.257223563,
            AUTHORITY["EPSG","7030"]],
        AUTHORITY["EPSG","6326"]],
    PRIMEM["Greenwich",0],
    UNIT["degree",0.0174532925199433],
    AUTHORITY["EPSG","4326"]]
Origin = (-20.306601000000001,-24.709499000000001)
Pixel Size = (0.010000000000000,-0.010000000000000)
Metadata:
  AREA_OR_POINT=Area
Image Structure Metadata:
  INTERLEAVE=BAND
Corner Coordinates:
Upper Left  ( -20.3066010, -24.7094990) ( 20d18'23.76"W, 24d42'34.20"S)
Lower Left  ( -20.3066010, -43.2394990) ( 20d18'23.76"W, 43d14'22.20"S)
Upper Right (   1.6433990, -24.7094990) (  1d38'36.24"E, 24d42'34.20"S)
Lower Right (   1.6433990, -43.2394990) (  1d38'36.24"E, 43d14'22.20"S)
Center      (  -9.3316010, -33.9744990) (  9d19'53.76"W, 33d58'28.20"S)
Band 1 Block=2195x1 Type=Float32, ColorInterp=Gray
  NoData Value=0

*/


/* 
  To cite GDAL/OGR in publications use:

  GDAL/OGR contributors (2020). GDAL/OGR Geospatial Data Abstraction
  software Library. Open Source Geospatial Foundation. URL https://gdal.org

A BibTeX entry for LaTeX users is

  @Manual{,
    title = {{GDAL/OGR} Geospatial Data Abstraction software Library},
    author = {{GDAL/OGR contributors}},
    organization = {Open Source Geospatial Foundation},
    year = {2020},
    url = {https://gdal.org},
  }
 */