/******************************************************************************
 * Project:  NextGIS Formbuilder
 * Purpose:  formbuilder's project and its basic implementations
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

#ifndef PROJECT_H
#define PROJECT_H

#include <QList>
#include <QMap>
#include <QString>

// Note: do not use GDAL < 2.1.0 in program at all because of vcizip bug, see
// https://trac.osgeo.org/gdal/ticket/6005
#include "ogrsf_frmts.h"
#include "json/json.h"

#include "fb_general.h"
#include "form.h"

// TODO: think about how to set these constants via build system:
#define _FB_GDAL_DEBUG "D:/nextgis/formbuilder/gdal-log.txt" // uncomment for GDAL
                                                             // debug output
#define _FB_INSTALLPATH_GDALDATA "/gdal_data"

// Names.
#define FB_PROJECT_EXTENSION "ngfp"
#define FB_PROJECT_FORM_FILENAME "form.json"
#define FB_PROJECT_DATA_FILENAME "data.geojson"
#define FB_PROJECT_META_FILENAME "meta.json"

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


struct FBFieldDescr // see NextGISWeb fields description syntax
{
    QString datataype;
    QString display_name;
    FBFieldDescr () {}
    FBFieldDescr (QString datataype,QString display_name)
        {this->datataype=datataype; this->display_name=display_name;}
    //~FBFieldDescr();
};

struct FBNgwConnection
{
    int id;
    QString login;
    QString password;
    QString url;
    FBNgwConnection () {}
    FBNgwConnection (int id, QString login, QString password, QString url)
        {this->id=id; this->login=login, this->password=password; this->url=url;}
    //~FBNgwConnection();
};


/**
 * Project class which represents its file on disk (.ngfp file).
 * Project contains 1) form, 2) layer (data) and 3) metadata.
 * Note: this class does not keep the layer's data in memory (it
 * will be accessed on disk each time) and the form itself (it is owned
 * by the screen widget), but it is able to read/write them into project
 * file.
 *
 * Project types differ from each other only in creation and first saving
 * capabilities, because other work with project is just working with
 * .ngfp files - which is always the same.
 */
class FBProject
{
    public: // static members

     static void init ();
     static QStringList DATA_TYPES; // see NextGISWeb types syntax
     static QStringList GEOM_TYPES;
     static QList<int> SRS_TYPES;
     static QString CUR_ERR_INFO; // additional error text if was some

    public: // methods

     FBProject ();
     virtual ~FBProject ();

     // main methods
     virtual FBErr create (QString anyPath) { return FBErrUnsupported; }
     virtual FBErr saveFirst (QString ngfpFullPath,
                              FBForm *formPtr) { return FBErrUnsupported; }
     FBErr saveAs (QString ngfpFullPath, FBForm *formPtr);
     FBErr save (FBForm *formPtr);
     FBErr open (QString ngfpFullPath);
     Json::Value readForm (QString ngfpFullPath);
     Json::Value readMeta (QString ngfpFullPath);
     bool checkData (QString ngfpFullPath);

     // info
     bool wasFirstSaved ();
     bool isSaveRequired () ;

    protected: // methods

//     FBErr writeForm (FBForm *formPtr);
//     FBErr writeMeta (QString strPathNgfp);

    protected: // fields
     
     // current state and link to the disk representation - i.e. ngfp file
     bool isInited; // whether the project had been already created or opened
     QString strNgfpPath; // if path is void - project needs to be saved first time

     // project's images
//     QList<QImage> images;

     // project's metadata
     QMap<QString,FBFieldDescr> fields; // keyname is a unique key in the map
     QString geometry_type;
     FBNgwConnection ngw_connection; // it is here because of ngfp file syntax
     int srs;
     QString version;
};


/*
class FBProjectVoid: public FBProject
{
    public:
     FBProjectVoid (QString geometry_type);
     virtual ~FBProjectVoid ();
     virtual FBErr create (QString anyPath);
     virtual FBErr saveFirst (QString ngfpFullPath, FBForm *formPtr);
};

class FBProjectGDAL: public FBProject
{
    public:
     FBProjectGDAL ();
     virtual ~FBProjectGDAL ();
};

class FBProjectShapefile: public FBProjectGDAL
{
    public:
     FBProjectShapefile ();
     virtual ~FBProjectShapefile ();
     virtual FBErr create (QString anyPath);
     virtual FBErr saveFirst (QString ngfpFullPath, FBForm *formPtr);
};

class FBProjectNGW: public FBProjectGDAL
{
    public:
     FBProjectNGW ();
     virtual ~FBProjectNGW ();
     virtual FBErr create (QString anyPath);
     virtual FBErr saveFirst (QString ngfpFullPath, FBForm *formPtr);
};
*/

#endif //PROJECT_H
