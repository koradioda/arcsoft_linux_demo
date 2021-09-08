#include "facesdatabase.h"
#include <QDebug>

FacesDataBase::FacesDataBase(QObject *parent) : QObject(parent)
{
    m_DataBase = QSqlDatabase::addDatabase("QSQLITE");          // 创建数据库类型
    //    m_DataBase.setDatabaseName("mFeature.db");                  // 设置数据库的名字
}

FacesDataBase::FacesDataBase(const QString &dbName)
{
    m_DataBase = QSqlDatabase::addDatabase("QSQLITE");          // 创建数据库类型
    m_DataBase.setDatabaseName(dbName);                         // 设置数据库的名字
    if(!m_DataBase.open())
        qDebug()<<m_DataBase.lastError()<<"对象文件除了问题啦！";                                          // 打开数据库
    m_Query = QSqlQuery(m_DataBase);                            // 绑定查询句柄
}

// 关闭数据库
FacesDataBase::~FacesDataBase()
{
    m_DataBase.close();         // 关闭数据库
    m_Query.clear();
}

bool FacesDataBase::createTable(const QString &createSql) // 创建指定的数据库
{
    return m_Query.exec(createSql);
}

bool FacesDataBase::createTable()   // 创建默认的 face_feature 数据库
{
    QString createSql = "CREATE TABLE IF NOT EXISTS face_feature (name text,featureSize int,feature BLOB,imagedata BLOB)";
    return m_Query.exec(createSql);
}

bool FacesDataBase::loadDataBase(QMap<QString, ASF_FaceFeature> &featureDB,QMap<QString,QPixmap> &pixMap)
{
    QString tsql = "select * from face_feature";   // 查询数据库所有数据，并返回保存到数据中
    if(m_Query.exec(tsql))
    {
        while(m_Query.next())
        {
            QString name = m_Query.value(0).toString();

            ASF_FaceFeature ff={0,0};
            ff.featureSize = m_Query.value(1).toInt();
            ff.feature = (MByte *)malloc(ff.featureSize);
            memset(ff.feature,0,ff.featureSize);
            // get feature data
            QByteArray data = m_Query.value(2).toByteArray();
            memcpy(ff.feature,reinterpret_cast<unsigned char*>(data.data()),ff.featureSize);
            featureDB.insert(name,ff);

            // deal image data
            QByteArray outByteArray = m_Query.value(3).toByteArray();
            QPixmap pix = QPixmap();
            pix.loadFromData(outByteArray);
            pixMap.insert(name,pix);
        }
        return true;    // 获取所有数据，成功返回true
    }
    else
        return false;
}

bool FacesDataBase::deleteItem(const QByteArray &feature)
{
    m_Query.prepare("delete from face_feature where feature= :feature ");
    m_Query.bindValue(":feature",feature);
    if(m_Query.exec())
        return true;
    else
        return false;
}

bool FacesDataBase::deleteTable()
{
    QString del_table_sql = "DROP TABLE face_feature";
    if(m_Query.exec(del_table_sql))
        return true;
    else
        return false;
}

bool FacesDataBase::deleteTable(const QString &tableName)
{
    m_Query.prepare("DROP TABLE :tableName ");
    m_Query.bindValue(":tableName",tableName);
    if(m_Query.exec())
        return true;
    else
        return false;
}

bool FacesDataBase::insertItem(const QString &pName,const int &fSize,const QByteArray &fData,const QByteArray &imgData)
{
    // 将数据保存到数据库默认表中
    m_Query.prepare("INSERT INTO face_feature (name,featureSize,feature,imagedata) VALUES(:name,:featureSize,:feature,:imagedata) ");
    m_Query.bindValue(":name",pName);                 //QString
    m_Query.bindValue(":featureSize",fSize); // int
    m_Query.bindValue(":feature",fData);             // QByteArray
    m_Query.bindValue(":imagedata",imgData);      // QByteArray

    return m_Query.exec()? true:false ;
}

bool FacesDataBase::insertItem(const QString tName, const QString &pName, const int &fSize, const QByteArray &fData, const QByteArray &imgData)
{
    // 将数据保存到数据库指定表中
    m_Query.prepare("INSERT INTO :tableName (name,featureSize,feature,imagedata) VALUES(:name,:featureSize,:feature,:imagedata) ");
    m_Query.bindValue(":name",tName);
    m_Query.bindValue(":name",pName);                 //QString
    m_Query.bindValue(":featureSize",fSize);          // int
    m_Query.bindValue(":feature",fData);              // QByteArray
    m_Query.bindValue(":imagedata",imgData);          // QByteArray

    return m_Query.exec()? true:false ;
}
