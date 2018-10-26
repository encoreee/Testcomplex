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

            Test & operator= (Test const & othertest)
            {
                m_testName = othertest.m_testName;
                return *this;
            }

        QString m_testName;
        QString m_fileName;
        QString m_directory;
        QStringList m_logData;
        QDateTime m_readingDateTime;

};

Q_DECLARE_METATYPE(Test)

#endif // TEST_H


