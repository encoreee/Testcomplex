#include "testingdialog.h"
#include "ui_testingdialog.h"
#include <QIntValidator>
#include <QLineEdit>

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");

TestingDialog::TestingDialog(QWidget *parent) :
    QDialog(parent),
    m_testui(new Ui::TestingDialog),
    m_intFrequencyValidator(new QIntValidator(0, 10000, this)),
    m_intCyclesNumberVaildator(new QIntValidator(0, 10000, this))

{
     m_testui->setupUi(this);

     connect(m_testui->createButton, &QPushButton::clicked, this, &TestingDialog::create);
     connect(m_testui->cancelButton, &QPushButton::clicked, this, &TestingDialog::cancel);
     connect(m_testui->frequencyBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TestingDialog::checkCustomPollingFrequency);
     connect(m_testui->cyclesBox,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TestingDialog::checkCustomCustomCyclesNumber);
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


TestSettings TestingDialog::createTest()
{
    TestSettings newSettings;

    if (m_testui->frequencyBox->currentIndex() == 13)
    {
        newSettings.pollingFrequency = m_testui->frequencyBox->currentText().toInt();
    }
    else
    {
       newSettings.pollingFrequency = m_testui->frequencyBox->itemData(m_testui->frequencyBox->currentIndex()).toInt();
    }


    if (m_testui->cyclesBox->currentIndex() == 7)
    {
        newSettings.numberOfCycles = m_testui->cyclesBox->currentText().toInt();
    }
    else
    {
       newSettings.numberOfCycles = m_testui->cyclesBox->itemData(m_testui->cyclesBox->currentIndex()).toInt();
    }

    if(m_testui->snrButton->isChecked())
        newSettings.choosenTest = Tests::SNR;

    if(m_testui->warmingupButton->isChecked())
        newSettings.choosenTest = Tests::WarmingUp;

    if(m_testui->crushButton->isChecked())
        newSettings.choosenTest = Tests::Crush;

    return newSettings;
}

void TestingDialog::checkCustomCustomCyclesNumber(int idx)
{
    const bool isCustomCyclesNumber = !m_testui->cyclesBox->itemData(idx).isValid();
    m_testui->cyclesBox->setEditable(isCustomCyclesNumber);
    if (isCustomCyclesNumber) {
        m_testui->cyclesBox->clearEditText();
        QLineEdit *edit = m_testui->cyclesBox->lineEdit();
        edit->setValidator(m_intCyclesNumberVaildator);
    }

}

void TestingDialog::checkCustomPollingFrequency(int idx)
{
    const bool isCustomPollingFrequency = !m_testui->frequencyBox->itemData(idx).isValid();
    m_testui->frequencyBox->setEditable(isCustomPollingFrequency);
    if (isCustomPollingFrequency) {
        m_testui->frequencyBox->clearEditText();
        QLineEdit *edit = m_testui->frequencyBox->lineEdit();
        edit->setValidator(m_intFrequencyValidator);
    }
}

void TestingDialog::updateSettings()
{

}

void TestingDialog::fillParameters()
{
    m_testui->frequencyBox->addItem(QStringLiteral("100"), 100);
    m_testui->frequencyBox->addItem(QStringLiteral("200"), 200);
    m_testui->frequencyBox->addItem(QStringLiteral("300"), 300);
    m_testui->frequencyBox->addItem(QStringLiteral("400"), 400);
    m_testui->frequencyBox->addItem(QStringLiteral("500"), 500);
    m_testui->frequencyBox->addItem(QStringLiteral("600"), 600);
    m_testui->frequencyBox->addItem(QStringLiteral("700"), 700);
    m_testui->frequencyBox->addItem(QStringLiteral("800"), 800);
    m_testui->frequencyBox->addItem(QStringLiteral("900"), 900);
    m_testui->frequencyBox->addItem(QStringLiteral("1000"), 1000);
    m_testui->frequencyBox->addItem(QStringLiteral("1500"), 1500);
    m_testui->frequencyBox->addItem(QStringLiteral("2000"), 2000);
    m_testui->frequencyBox->addItem(QStringLiteral("3000"), 3000);

    m_testui->frequencyBox->addItem(tr("Custom"));
    m_testui->frequencyBox->setCurrentIndex(2);

    m_testui->cyclesBox->addItem(QStringLiteral("50"), 50);
    m_testui->cyclesBox->addItem(QStringLiteral("100"), 100);
    m_testui->cyclesBox->addItem(QStringLiteral("250"), 250);
    m_testui->cyclesBox->addItem(QStringLiteral("500"), 500);
    m_testui->cyclesBox->addItem(QStringLiteral("750"), 750);
    m_testui->cyclesBox->addItem(QStringLiteral("1000"), 1000);
    m_testui->cyclesBox->addItem(QStringLiteral("1500"), 1500);

    m_testui->cyclesBox->addItem(tr("Custom"));
    m_testui->cyclesBox->setCurrentIndex(0);
}
