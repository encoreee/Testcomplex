#include "test.h"

Test::Test()
{
    m_testName = "";
}
Test::~Test()
{

}

void Test::setTestName(QString testName)
{
  m_testName = testName;
}
void Test::cleanData()
{
    m_testName  =  "";
    m_fileName  =  "";
    m_directory = "";;
    m_logData.clear();
    m_readingDateTime = QDateTime();
    isEmpty = true;
}
