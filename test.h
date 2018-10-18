#ifndef TEST_H
#define TEST_H
#include <QList>
#include <QString>
#include <QStringList>
#include <QDateTime>

class Test
{
    public:
        void setTestName(QString testName);

        Test();
        ~Test();
        QString m_testName;
        QList <QString> m_fileName;
        QList <QString> m_directory;
        QList <QStringList> m_logData;
        QList <QDateTime> m_readingDateTime;

};


#endif // TEST_H
