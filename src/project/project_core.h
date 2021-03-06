/******************************************************************************
 * Project:  NextGIS Formbuilder
 * Purpose:  project core definitions
 * Author:   Mikhail Gusev, gusevmihs@gmail.com
 ******************************************************************************
*   Copyright (C) 2014-2016 NextGIS
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ****************************************************************************/

#ifndef PROJECT_CORE_H
#define PROJECT_CORE_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QFile>

#include "ogrsf_frmts.h"
#include "json/json.h"

//#define FB_PATH_GDALDATA "../share/nextgis/fb/gdal"
#define FB_PATH_GDALDATA "../share/gdal"

// The only float version constant of all Formbuilder project.
#define FB_VERSION 2.2

// Names.
#define FB_PROJECT_EXTENSION "ngfp"
#define FB_PROJECT_FILENAME_FORM "form.json"
#define FB_PROJECT_FILENAME_DATA "data.geojson"
#define FB_PROJECT_FILENAME_META "meta.json"

// Keys for JSON.
// Change this carefully, or do not change this in future versions! These keys are
// written in older versions of .ngfp files.
#define FB_JSON_META_VERSION "version"
#define FB_JSON_META_FIELDS "fields"
#define FB_JSON_META_KEYNAME "keyname"
#define FB_JSON_META_DATATYPE "datatype"
#define FB_JSON_META_DISPLAY_NAME "display_name"
#define FB_JSON_META_GEOMETRY_TYPE "geometry_type"
#define FB_JSON_META_NGW_CONNECTION "ngw_connection"
#define FB_JSON_META_SRS "srs"
#define FB_JSON_META_ID "id"
#define FB_JSON_META_LOGIN "login"
#define FB_JSON_META_PASSWORD "password"
#define FB_JSON_META_URL "url"
#define FB_JSON_META_LISTS "lists"
#define FB_JSON_META_KEY_LIST "key_list"

// Indexes for named items in the main array of types.
#define FB_TYPEINDEX_DATA_STRING 0
#define FB_TYPEINDEX_DATA_INTEGER 1
#define FB_TYPEINDEX_SRS_WGS84 0

// Error codes.
enum FBErr
{
    FBErrNone,

    // FOR DEVELOPERS: add string representations of new error codes to the
    // FB class.
    FBErrWrongVersion, FBErrIncorrectJson, FBErrIncorrectFileStructure,
    FBErrBadSavePath, FBErrTempFileFail, FBErrGDALFail, FBErrCPLFail,
    FBErrReadNgfpFail, FBErrStructureDiffers, FBErrIncorrectGdalDataset,

    // Says that for ngw dataset initializing it is not an error, but for others
    // as FBErrIncorrectGdalDataset.
    FBErrIncorrectGdalDataset_NotForNgw,

    // For the following codes it is not necessary to show detailed error info.
    // WARNING. It is always necessary to check for this type when displaying error
    // in GUI because the old info can still hang in the global error variable.
    FBErrNullVal, FBErrNotInited, FBErrNoNgfp, FBErrAlreadyInited, FBErrUnsupported,
};

// Types correspondance.
struct FbGeomType
{
    QString aliasNgw;
    OGRwkbGeometryType aliasGdal;
// QList<OGRwkbGeometryType> aliasesOtherGdal; // Polygon = Polygon25, Polygon3D, ...
    QString strDisplayName;
    FbGeomType (QString an, OGRwkbGeometryType ad, QString sdn)
    {
        aliasNgw = an;
        aliasGdal = ad;
        strDisplayName = sdn;
    }
    ~FbGeomType () { }
};

struct FbDataType
{
    QString aliasNgw;
    OGRFieldType aliasMainGdal;
    QList<OGRFieldType> aliasesOtherGdal;
    FbDataType (QString an, OGRFieldType amg)
    {
        aliasNgw = an;
        aliasMainGdal = amg;
    }
    FbDataType (QString an, OGRFieldType amg, QList<OGRFieldType> aog)
    {
        aliasNgw = an;
        aliasMainGdal = amg;
        aliasesOtherGdal = aog;
    }
    ~FbDataType () { }
};

struct FbSrsType
{
    int numberNgw;
    QString strFullGdal;
    FbSrsType (int nn, QString sfg)
    {
        numberNgw = nn;
        strFullGdal = sfg;
    }
    ~FbSrsType () { }
};

// Main metadata structures:

struct FBField
{ // see NextGISWeb fields description syntax
     FbDataType *datataype;
     QString display_name;
     FBField ()
     {
         datataype = NULL;
     }
     FBField (FbDataType *dt, QString dn)
     {
         datataype = dt;
         display_name = dn;
     }
     //~FBField();
     bool operator==(const FBField &other) const
     {
         return ((other.datataype == datataype)
           && (QString::compare(other.display_name, display_name,
                               Qt::CaseInsensitive) == 0));
     }
     bool operator!=(const FBField &other) const
     {
         return !(*this == other);
     }
};


struct FBNgwConnection
{
    int id; // = -1 if no NGW connection
    QString login;
    QString password;
    QString url;
    FBNgwConnection () { }
    FBNgwConnection (int i, QString lg, QString ps, QString ul)
    {
        id = i;
        login = lg;
        password = ps;
        url = ul;
    }
    //~FBNgwConnection();
};


/**
 * Project class which represents its file on disk (.ngfp file).
 * Project file contains 1) form, 2) layer (data) and 3) metadata.
 * Note: this class does not keep the layer's data in memory (it
 * will be accessed on disk each time) and the form itself (it is owned
 * by the screen widget), but it is able to read/write them into project
 * file.
 *
 * FOR DEVELOPERS: Concrete project types differ from each other only in
 * creation and first-time saving capabilities, because other work with project
 * is just working with .ngfp files - which is always the same and implemented
 * in base class.
 */
class FBProject: public QObject
{
    Q_OBJECT // only for showing progress for long operations

    public: // static global members

     // General lists of types correspondance.
     // WARNING. Only for reading outside. Define new types in init() method.
     static QList<FbGeomType*> GEOM_TYPES;
     static QList<FbDataType*> DATA_TYPES;
     static QList<FbSrsType*> SRS_TYPES;

     static QString CUR_ERR_INFO; // additional error text if was some

     static void initEnv ();
     static void deinitEnv ();

     static FbGeomType *findGeomTypeByNgw (QString aliasNgw);
     static FbGeomType *findGeomTypeByGdal (OGRwkbGeometryType aliasGdal);
     static FbDataType *findDataTypeByNgw (QString aliasNgw);
     static FbDataType *findDataTypeByGdal (OGRFieldType aliasGdal);
     static FbSrsType *findSrsTypeByNgw (int numberNgw);

    signals:

     void changeProgress (int percentage); // amount of work completed for long actions

    public:

     FBProject ();
     virtual ~FBProject ();

     // main I/O
     virtual FBErr readFirst (QString anyPath) { return FBErrUnsupported; }
     FBErr read (QString ngfpFullPath);
     static Json::Value readMeta (QString ngfpFullPath);
     static Json::Value readForm (QString ngfpFullPath);
     static bool readData (QString ngfpFullPath);
     FBErr write (QString ngfpFullPath, Json::Value jsonForm);
     FBErr write (Json::Value jsonForm);
     FBErr writeData (QString shapefilePath);

     // modify
     void modifyFields (QMap<QString,FBField> newSetOfFields,
                                QSet<QString> fieldsWereDeleted);
     void updateLists (QList<QStringList> newLists, QString newKeyList);
     // info
     static QString getProgVersionStr ();
     static void getPathComponents (QString strFullPath, QString &strPath,
                             QString &strNameBase);
     Json::Value getJsonMetadata ();
     bool wasInited () { return isInited; }
     bool wasFirstSaved () { return strNgfpPath != ""; }
     bool isSaveRequired ();
     bool needToSaveFieldsModifics () { return fieldsModified; }
     QString getCurrentFilePath () { return strNgfpPath; }
     virtual QString getDatasetPath () { return ""; }
     QMap<QString,FBField> getFields () { return fields; }
     FbGeomType *getGeomType () { return geometry_type; }
     FBNgwConnection getNgwConnection () { return ngw_connection; }
     QList<QStringList> getLists () { return lists; }
     QString getKeyList () { return keyList; }

    protected:

     // inner I/O methods
     virtual FBErr writeDataFileFirst (QString strPath);
     FBErr writeDataFile (GDALDataset *datasetSrcPtr, QString strPath,
                          bool ignoreFieldsModifs = false);
     static Json::Value readJsonInNgfp (QString ngfpFullPath, QString fileName);
     //static QList<QImage> readImagesInNgfp (QString ngfpFullPath);
     static FBErr writeJsonFile (QFile &file, Json::Value json);

    protected:

     bool isInited; // whether the project had been already initialized (read). If the
                    // project is inited - it MUST HAVE ALL ITS METADATA SET

     // Current link to the project's representation on disk - i.e. ngfp file.
     QString strNgfpPath; // if path is void (no ngfp file yet) - project needs to
                          // be saved first time
     // project's images
//     QList<QImage> images;

     // project's metadata
     QMap<QString,FBField> fields; // keyname is a unique key in the map
     FbGeomType *geometry_type;
     FBNgwConnection ngw_connection; // it is here because of ngfp file syntax
     FbSrsType *srs;
     QString version;
     QList<QStringList> lists;
     QString keyList;

     // temporary. Reseted after the call of according method
     bool fieldsModified;
     QSet<QString> fieldsDeleted; // stored keynames of fields

    private: // some common actions

     QList<QPair<QString,QString> > getInnerFilePaths (QString tempPath);
     FBErr makeZip (QString ngfpTempPath,
                      QList<QPair<QString,QString> > strsTempFiles);
     static bool addFileToZip (const CPLString &szFilePathName, const CPLString
                   &szFileName, void* hZIP, GByte **pabyBuffer, size_t nBufferSize);
     FBErr modifyFieldsOfLayer (OGRLayer *layer);
     FBErr reprojectLayer (OGRLayer *layer);
};


#endif //PROJECT_CORE_H
