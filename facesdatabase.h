#ifndef FACESDATABASE_H
#define FACESDATABASE_H

#include <QObject>
#include <QSqlDatabase> // sqlite 头文件
#include <QSqlError>    // 数据库错误信息文件
#include <QSqlQuery>    // sql 查询语句包
#include <QMap>
#include <inc/arcsoft_face_sdk.h>
#include <QPixmap>
#include <QVariant>

class FacesDataBase : public QObject
{
    Q_OBJECT
public:
    explicit FacesDataBase(QObject *parent = nullptr);
    FacesDataBase(const QString &dbName);
    ~FacesDataBase();

public:
    bool createTable(const QString &createSql);         // 创建自己的表的名字
    bool createTable();
    bool loadDataBase(QMap<QString,ASF_FaceFeature> &featureDB,QMap<QString,QPixmap> &pixMap);
    bool deleteItem(const QByteArray &feature );   // 删除默认表指定的数据
    bool deleteTable();                             // 删除默认表
    bool deleteTable(const QString &tableName);     // 删除指定的表
    bool insertItem(const QString &pName,const int &fSize,const QByteArray &fData,const QByteArray &imgData);                             // 插入数据到默认表的数据
    bool insertItem(const QString tName,const QString &pName,const int &fSize,const QByteArray &fData,const QByteArray &imgData);                             // 插入数据到指定的数据

signals:

private:
    QSqlDatabase m_DataBase;            // 数据库句柄
    QSqlQuery m_Query;                  // 查询语句句柄

};

#endif // FACESDATABASE_H
