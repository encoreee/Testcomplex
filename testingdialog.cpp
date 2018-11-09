#include "testingdialog.h"
#include "ui_testingdialog.h"
#include <QIntValidator>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

TestingDialog::TestingDialog(QWidget *parent) :
    QDialog(parent),
    m_testui(new Ui::TestingDialog),
    m_intTestValidator(new QIntValidator(0, 4000000, this))

{
     m_testui->setupUi(this);

     connect(m_testui->createButton, &QPushButton::clicked, this, &TestingDialog::create);
     connect(m_testui->cancelButton, &QPushButton::clicked, this, &TestingDialog::cancel);
     fillParameters();
}

TestingDialog::~TestingDialog()
{
    delete m_testui;
}

bool TestingDialog::create()
{

    accept();
    return true;
}

bool TestingDialog::cancel()
{
    reject();
    return false;
}


TestingDialog::TestSettings TestingDialog::createTest()
{
    TestSettings newSettings;
    return newSettings;
}

void TestingDialog::updateSettings()
{

}

void TestingDialog::fillParameters()
{
    m_testui->frequencyBox->addItem(QStringLiteral("500"), 500);
    m_testui->frequencyBox->addItem(QStringLiteral("1000"), 1000);
    m_testui->frequencyBox->addItem(QStringLiteral("1500"), 1500);
    m_testui->frequencyBox->addItem(QStringLiteral("2000"), 2000);
    m_testui->frequencyBox->addItem(QStringLiteral("2500"), 2500);
    m_testui->frequencyBox->addItem(QStringLiteral("5000"), 5000);

    m_testui->frequencyBox->addItem(tr("Custom"));
    m_testui->frequencyBox->setCurrentIndex(2);
}
