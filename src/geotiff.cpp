/**
 * @file geotiff.cpp
 * @author Jose Cappelletto (cappelletto@gmail.com)
 * @brief Provides a simple interface to GDAL API fo reading geoTIFF files
 * @version 0.1
 * @date 2020-07-03
 * @copyright Copyright (c) 2020
 * @url Based on https://gerasimosmichalitsianos.wordpress.com/2018/11/30/431/
 * @author Gerasimos Michalitsianos
 * */

#include <geotiff.hpp>
// GDAL specific libraries
#include <gdal_priv.h>
#include <cpl_conv.h> // for CPLMalloc()

#include <iostream>
#include <string>
#include <gdal_priv.h>
#include <cpl_conv.h>
#include <gdalwarper.h>
#include <stdlib.h>
 
/**
 * @brief This function returns the filename of the Geotiff
 * 
 * @return const char* 
 */
const char *Geotiff::GetFileName() { 
    return filename; 
  }
 
double Geotiff::GetNoDataValue() { 
  /* 
      * function GetNoDataValue(): 
      *  This function returns the NoDataValue for the Geotiff dataset. 
      *  Returns the NoData as a double. 
      */
  GDALDataset *dataset; // Geotiff GDAL datset object. 
  dataset = geotiffDataset;
  // int f = dataset->GetRasterCount(); // \todo: cheack that we have at least one band
  GDALRasterBand *hBand = dataset->GetRasterBand(1);
  double dfNoData = (double)geotiffDataset->GetRasterBand(1)->GetNoDataValue();
  return dfNoData;
  }

/**
 * @brief Returns the geotiff file projection as string
 * @details This function returns a character array (string) 
 *  for the projection of the geotiff file. Note that
 *  the "->" notation is used. This is because the 
 *  "geotiffDataset" class variable is a pointer 
 *  to an object or structure, and not the object
 *  itself, so the "." dot notation is not used. 
 * @return const char* 
 */
const char *Geotiff::GetProjection() { 
    return geotiffDataset->GetProjectionRef(); 
  } 

/**
 * @brief Returns a pointer to the 6-D geo-transformation of the geotiff file 
 * @details This function returns a pointer to a double that 
 *  is the first element of a 6 element array that holds
 *  the geotransform of the geotiff.  
 * 
 * @return double* 
 */
double *Geotiff::GetGeoTransform() {
  geotiffDataset->GetGeoTransform(geotransform);
  return geotransform; 
} 
     
float** Geotiff::GetRasterBand(int z) {
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
  // cout << "[Geotiff] Creating data container" << endl;
  float** bandLayer = new float*[nRows];
  switch( GDALGetRasterDataType(geotiffDataset->GetRasterBand(z)) ) {
      case 0:
      return NULL; // GDT_Unknown, or unknown data type.
      case 1:
      // GDAL GDT_Byte (-128 to 127) - unsigned  char
      return GetArray2D<unsigned char>(z,bandLayer); 
      case 2:
      // GDAL GDT_UInt16 - short
      return GetArray2D<unsigned short>(z,bandLayer);
      case 3:
      // GDT_Int16
      return GetArray2D<short>(z,bandLayer);
      case 4:
      // GDT_UInt32
      return GetArray2D<unsigned int>(z,bandLayer);
      case 5:
      // GDT_Int32
      return GetArray2D<int>(z,bandLayer);
      case 6:
      // GDT_Float32
      return GetArray2D<float>(z,bandLayer);
      case 7:
      // GDT_Float64
      return GetArray2D<double>(z,bandLayer);
      default:     
      break;  
  }
  return NULL;  
}

GDALDataset *Geotiff::GetDataset(){
  return geotiffDataset;  //return pointer to the main GDAL TIFF dataset, if user want to use GDAL C++ API directly
  // this breaks the abstraction layer, but we do not want to isolate the whole API
}

void Geotiff::GetDimensions(int *dim){
  dim[0] = nCols;
  dim[1] = nRows;
  dim[2] = nBands;
}

int *Geotiff::GetDimensions() {
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
  dimensions[0] = nCols; 
  dimensions[1] = nRows;
  dimensions[2] = nBands; 
  return dimensions;  
} 

template<typename T>
float** Geotiff::GetArray2D(int layerIndex,float** bandLayer) {

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
    GDALDataType bandType = GDALGetRasterDataType(geotiffDataset->GetRasterBand(layerIndex));

    // get number of bytes per pixel in Geotiff
    int nbytes = GDALGetDataTypeSizeBytes(bandType);

    // cout << "[GetArray2D] Allocating rowBuff" << endl;    
    // allocate pointer to memory block for one row (scanline) 
    // in 2D Geotiff array.  
    T *rowBuff = (T*) CPLMalloc(nbytes*nCols);

    // cout << "[GetArray2D] Allocating rows:" << nRows << endl;    
    for(int row=0; row<nRows; row++) {     // iterate through rows

      // cout << "Row:\t" << row << endl;
      // read the scanline into the dynamically allocated row-buffer       
      CPLErr e = geotiffDataset->GetRasterBand(layerIndex)->RasterIO(GF_Read,0,row,nCols,1,rowBuff,nCols,1,bandType,0,0);
      if(!(e == 0)) { 
        cout << "[geotiff] Error: Unable to read scanline in Geotiff!" << endl;
        exit(1);
      }
        
      bandLayer[row] = new float[nCols];
      for( int col=0; col<nCols; col++ ) { // iterate through columns
        bandLayer[row][col] = (float)rowBuff[col];
      }
    }
    CPLFree( rowBuff );
    return bandLayer;
}

bool Geotiff::isValid(){
  return bValidDataset;
}
 
void Geotiff::ShowInformation(){
  /*
  * function void ShowInformation()
  * This function prints out a summary of current dataset variables
  */
	double adfGeoTransform[6];
	cout << "Driver:\t\t" << geotiffDataset->GetDriver()->GetDescription() << "/" << geotiffDataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) << endl;
	cout << "Size is\tX: " << geotiffDataset->GetRasterXSize() << "\tY: " << geotiffDataset->GetRasterYSize() << "\tC: " << geotiffDataset->GetRasterCount() << endl; 
	if( geotiffDataset->GetProjectionRef()  != NULL )
	    cout << "Projection is " << geotiffDataset->GetProjectionRef() << endl;
	if( geotiffDataset->GetGeoTransform( adfGeoTransform ) == CE_None )
	{
	    cout << "Origin =\t" <<  adfGeoTransform[0] << ", " << adfGeoTransform[3] << endl;
	    cout << "Pixel Size =\t" << adfGeoTransform[1] << ", " << adfGeoTransform[5] << endl;
	}

  // for each available band, we print its information
	GDALRasterBand  *poBand;
	int             nBlockXSize, nBlockYSize;
	int             bGotMin, bGotMax;
	double          adfMinMax[2];

  for (int i=1; i<=nBands; i++){
    poBand = geotiffDataset->GetRasterBand(i);  // 1-indexed band number. We retrieve the first (and unique band)
    poBand->GetBlockSize( &nBlockXSize, &nBlockYSize );
    printf( "Block=%dx%d Type=%s, ColorInterp=%s\n",
        nBlockXSize, nBlockYSize,
        GDALGetDataTypeName(poBand->GetRasterDataType()),
        GDALGetColorInterpretationName(poBand->GetColorInterpretation()) );
    adfMinMax[0] = poBand->GetMinimum( &bGotMin );
    adfMinMax[1] = poBand->GetMaximum( &bGotMax );
    if( ! (bGotMin && bGotMax) )
      GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);
    cout << "Min = " << adfMinMax[0] <<",\tMax = " << adfMinMax[1] << endl;
    if( poBand->GetOverviewCount() > 0 )
      cout << "Band has " << poBand->GetOverviewCount() << "overviews" << endl;
    if( poBand->GetColorTable() != NULL )
      cout << "Band has a color table with " << poBand->GetColorTable()->GetColorEntryCount() << " entries" << endl;

    cout << "Units:\t\t" << poBand->GetUnitType() << endl;

    // WARNING: TODO: this conditional is valid only for the first band which is retrieved in the construction at creation time
    if (!bGotNodata){
      cout << "Current band does not provide explicit no-data field definition" << endl;
    }
    else{
      if (CPLIsNan(dfNoData)){ //test if provided NoData is NaN
        cout << "NoData value: NaN --> " << dfNoData << endl;
      }
      else{
        cout << "NoData value: " << dfNoData << endl;
      }
    }
  }
	//*/
	// NAMES AND ORDERING OF THE AXES
}

/**
 * @brief Returns the parameter retrieved from the 6D geotransform matrix corresponding to the raster band 1
 * 
 * @param paramID constant index identiying the parameter to be retrieved
 * @return double value of the parameter
 */
double Geotiff::GetGeoTransformParam(int paramID){
  if (paramID < 0 || paramID > 6){
    //TODO: we should throw an exception
    cout << "[geotiff] Geotiff::GetGeoTransformParam invalid paramID: [" << paramID << "]. It must be 0 <= paramId <=5" << endl;
    cout << "ocurred @ " << __FILE__ << ":" << __LINE__ << endl; 
    return 0;
  }
  return geotransform[paramID];
}

// template<typename T>
float* Geotiff::GetArray1D(int layerIndex,float* bandLayer) {
    // get the raster data type (ENUM integer 1-12, 
    // see GDAL C/C++ documentation for more details)        
    GDALDataType bandType = GDALGetRasterDataType(geotiffDataset->GetRasterBand(layerIndex));
    
    // get number of bytes per pixel in Geotiff
    int nbytes = GDALGetDataTypeSizeBytes(bandType);

    // allocate pointer to memory block for one row (scanline) 
    // in 2D Geotiff array.  
    float *dataBuff = (float *) CPLMalloc(nbytes*nCols*nRows);

    CPLErr e = geotiffDataset->GetRasterBand(layerIndex)->RasterIO(GF_Read,0,0,nCols,nRows,dataBuff,nCols,nRows,bandType,0,0);
    if(!(e == 0)) { 
      cout << "Warning: Unable to read scanline in Geotiff!" << endl;
      exit(1);
    } 
    bandLayer = new float[nCols*nRows];
    for( int i=0; i<nCols*nRows; i++ ) { // iterate through columns
      bandLayer[i] = (float)dataBuff[i];
    }
    CPLFree( dataBuff );
    return bandLayer;
}