#ifndef TEST_H
#define TEST_H
#include <QList>
#include <QString>
#include <QStringList>
#include <QDateTime>
#include "testingdialog.h"

class Test
{
    public:

        void setTestName(QString testName);

        Test();

        ~Test();

              const Test & operator= ( Test const &othertest)
            {
                m_testName        = othertest.m_testName;
                m_fileName        = othertest.m_fileName;
                m_directory       = othertest.m_directory;
                m_logData         = othertest.m_logData;
                m_readingDateTime = othertest.m_readingDateTime;
                m_settings        = othertest.m_settings;
                return *this;
            }

            Test & operator= ( Test&othertest)
            {
                m_testName        = othertest.m_testName;
                m_fileName        = othertest.m_fileName;
                m_directory       = othertest.m_directory;
                m_logData         = othertest.m_logData;
                m_readingDateTime = othertest.m_readingDateTime;
                m_settings        = othertest.m_settings;

                return *this;
            }

        QString       m_testName;
        QString       m_fileName;
        QString       m_directory;
        QStringList   m_logData;
        QDateTime     m_readingDateTime;
        bool          isEmpty             = true;
        TestSettings  m_settings;

        void cleanData();

};

Q_DECLARE_METATYPE(Test)

#endif // TEST_H


