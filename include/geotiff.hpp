#ifndef _GEOTIFF_HPP_
#define _GEOTIFF_HPP_

#include <iostream>
#include <string>
#include <gdal_priv.h>
#include <cpl_conv.h>
#include <gdalwarper.h>
#include <stdlib.h>
#include <ogr_spatialref.h>

using namespace std;
typedef std::string String; 

#define GEOTIFF_PARAM_CX 0
#define GEOTIFF_PARAM_CY 3
#define GEOTIFF_PARAM_SX 1
#define GEOTIFF_PARAM_SY 5

class Geotiff { 
 
  private: // NOTE: "private" keyword is redundant here.  
           // we place it here for emphasis. Because these
           // variables are declared outside of "public", 
           // they are private. 
 
    const char* filename;        // name of Geotiff
    GDALDataset *geotiffDataset; // Geotiff GDAL datset object. 
    double geotransform[6];      // 6-element geotranform array.
    int dimensions[3];           // X,Y, and Z dimensions. 
    int nRows,nCols,nBands;     // dimensions of data in Geotiff. nBands:nChannels
    double dfNoData;
    int bValidDataset;
    int bGotNodata;
	// TODO: deal with GMF_NODATA & Masks
	// See: https://gdal.org/development/rfc/rfc15_nodatabitmask.html#rfc-15


  public: 
     
    // define constructor function to instantiate object
    // of this Geotiff class. 
    Geotiff( const char* tiffname ) { 
      filename = tiffname ; 
      GDALAllRegister();
      bValidDataset = true;
 
      // set pointer to Geotiff dataset as class member.  
      geotiffDataset = (GDALDataset*) GDALOpen(filename, GA_ReadOnly);
      if (geotiffDataset == NULL){
        // some problem ocurred reading the file. Flag it
        cout << "[geotiff] Error reading file: " << filename << endl;
        cout << "[geotiff] setting geotiffDataset = NULL" << endl;
        bValidDataset = false;
        return;
      }
      // set the dimensions of the Geotiff 
      nRows   = GDALGetRasterYSize( geotiffDataset ); 
      nCols   = GDALGetRasterXSize( geotiffDataset ); 
      nBands = GDALGetRasterCount( geotiffDataset );
      if (nBands < 1){
        cout << "[Geotiff::Geotiff] Retrieved invalid number of bands from geotiffDataset (" << __FILE__ << "@" << __LINE__ << endl;
      }
      if (nBands > 1){
        cout << "[Geotiff::Geotiff] More than 1 band found in the geotiff file. We will use only the first one" << endl;
        cout << "[Geotiff::Geotiff] multiband import not supported yet" << endl;
      }
      // retrieve, if available, no-data definition for the first band
      // BLUEPRINT: vector of no-data definitions. We could have 1 per band. How common is that?
      dfNoData = GDALGetRasterNoDataValue (GDALGetRasterBand( geotiffDataset, 1 ), &bGotNodata);
      geotiffDataset->GetGeoTransform(geotransform);
      // WIP: Retrieve Spatial Ref an populate local container;
      datasetSpatialRef = new OGRSpatialReference(geotiffDataset->GetProjectionRef());
    }
 
    // define destructor function to close dataset, 
    // for when object goes out of scope or is removed
    // from memory. 
    ~Geotiff() {
      // close the Geotiff dataset, free memory for array.  
      delete datasetSpatialRef; // free locally stored copy of OGRSpatialReference
      GDALClose(geotiffDataset);
    }

    double GetGeoTransformParam(int paramID); //returns value of single geoTransform parameter for RasterBand(1)

    OGRSpatialReference *datasetSpatialRef; //Dataset-wide OGR Spatial Ref (WKT format)

    GDALDataset *GetDataset();
    /*
     * function GDALDataset *GetDataset()
     * Returns pointer to Geotiff GDAL dataset object
     * 
     */ 
 
    void ShowInformation();
    /*
     * function void ShowInformation()
     * This function prints out a summary of current dataset variables
     */

    bool isValid();
    /*
     * function bool isValid()
     * This function returns if a valid geotiffDataset is available (e.g. succesfully read a TIFF file) 
     */
    const char *GetFileName();
      /* 
       * function GetFileName()
       * This function returns the filename of the Geotiff. 
       */
 
    const char *GetProjection();
      /* function const char* GetProjection(): 
       *  This function returns a character array (string) 
       *  for the projection of the geotiff file. Note that
       *  the "->" notation is used. This is because the 
       *  "geotiffDataset" class variable is a pointer 
       *  to an object or structure, and not the object
       *  itself, so the "." dot notation is not used. 
       */
 
    double *GetGeoTransform();
      /* 
       * function double *GetGeoTransform() 
       *  This function returns a pointer to a double that 
       *  is the first element of a 6 element array that holds
       *  the geotransform of the geotiff.  
       */

//    double GetNoDataValue(); 
      /* 
       * function GetNoDataValue(): 
       *  This function returns the NoDataValue for the Geotiff dataset. 
       *  Returns the NoData as a double. 
       */
 
    void GetDimensions(int *);

    int *GetDimensions();
      /* 
       * int *GetDimensions(): 
       * 
       *  This function returns a pointer to an array of 3 integers 
       *  holding the dimensions of the Geotiff. The array holds the 
       *  dimensions in the following order:
       *   (1) number of columns (x size)
       *   (2) number of rows (y size)
       *   (3) number of bands (number of bands, z dimension)
       */
 
    double GetNoDataValue(); 
    /* 
        * function GetNoDataValue(): 
        *  This function returns the NoDataValue for the Geotiff dataset. 
        *  Returns the NoData as a double. 
        */

    float** GetRasterBand(int z);
    /*
        * function float** GetRasterBand(int z): 
        * This function reads a band from a geotiff at a 
        * specified vertical level (z value, 1 ... 
        * n bands). To this end, the Geotiff's GDAL 
        * data type is passed to a switch statement, 
        * and the template function GetArray2D (see below)
        * is called with the appropriate C++ data type. 
        * The GetArray2D function uses the passed-in C++ 
        * data type to properly read the band data from 
        * the Geotiff, cast the data to float**, and return
        * it to this function. This function returns that 
        * float** pointer. 
        */

  
    template<typename T>
    float** GetArray2D(int layerIndex,float** bandLayer); 
       /*
        * function float** GetArray2D(int layerIndex): 
        * This function returns a pointer (to a pointer)
        * for a float array that holds the band (array)
        * data from the geotiff, for a specified layer 
        * index layerIndex (1,2,3... for GDAL, for Geotiffs
        * with more than one band or data layer, 3D that is). 
        *
        * Note this is a template function that is meant 
        * to take in a valid C++ data type (i.e. char, 
        * short, int, float), for the Geotiff in question 
        * such that the Geotiff band data may be properly 
        * read-in as numbers. Then, this function casts 
        * the data to a float data type automatically. 
        */
 
       // get the raster data type (ENUM integer 1-12, 
       // see GDAL C/C++ documentation for more details)         

    // template<typename T>
    float* GetArray1D(int layerIndex,float* bandLayer);

};

#endif