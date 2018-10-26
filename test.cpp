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

